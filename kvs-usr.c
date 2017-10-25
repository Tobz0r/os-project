/**File: Kvs-usr.c
**Author: Tobias Estefors, Thom Resntröm
**Destriction: User space program
* to add data to the lkm
*/
#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define NETLINK_USER  21
#define filename "results.txt"

#define MAX_SIZE 1024 /* maximum msg size*/

void write_to_file(char *str);
void delete_from_file(char *str);
int linecount(FILE* fp);
struct sockaddr_nl src, dest;
struct nlmsghdr *nlmsg = NULL;
struct iovec iov;
struct msghdr msg;


int main(int argc, char**argv){
    if(argc!=3 && argc!=4){
        printf("./kvs-usr Action key Value\n");
    }
    char type[2];
    if(strcmp(argv[1],"Add")==0){
        strcpy(type,"1");
    }else if (strcmp(argv[1],"Remove")==0){
        strcpy(type,"2");
    }else if (strcmp(argv[1],"Print")==0){
        strcpy(type,"3");
    }else{
        fprintf(stderr, "%s\n","Invalid action, use Add,Remove or Print" );
        exit(-1);
    }
    int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        return -1;

    memset(&src, 0, sizeof(src));
    src.nl_family = AF_NETLINK;
    src.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr *)&src, sizeof(src));

    memset(&dest, 0, sizeof(dest));
    memset(&dest, 0, sizeof(dest));
    dest.nl_family = AF_NETLINK;
    dest.nl_pid = 0; 
    dest.nl_groups = 0; /* unicast */

    /* setting up message*/
    nlmsg = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_SIZE));
    if(!nlmsg){
        perror("nlmsg");
        exit(-1);
    }
    memset(nlmsg, 0, NLMSG_SPACE(MAX_SIZE));
    nlmsg->nlmsg_len = NLMSG_SPACE(MAX_SIZE);
    nlmsg->nlmsg_pid = getpid();
    nlmsg->nlmsg_flags = 0;
    char msgstr[MAX_PAYLOAD];
    strcpy(msgstr,type);
    strcat(msgstr, " ");
    strcat(msgstr,argv[2]);
    if(strcmp(type,"1")==0){
        strcat(msgstr," ");
        strcat(msgstr,argv[3]);
    }

    strcpy(NLMSG_DATA(nlmsg), msgstr);

    iov.iov_base = (void *)nlmsg;
    iov.iov_len = nlmsg->nlmsg_len;
    msg.msg_name = (void *)&dest;
    msg.msg_namelen = sizeof(dest);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
;
    sendmsg(sock_fd, &msg, 0);
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    if(strcmp(type,"1")==0){
        write_to_file((char*)NLMSG_DATA(nlmsg));
    }
    else if(strcmp(type,"2")==0){
        char temp[MAX_PAYLOAD];
        strcpy(temp,argv[2]);
        printf("%s\n",temp );
        delete_from_file(temp);
    }else if(strcmp(type,"3")){
        printf("Received message: %s\n", (char*)NLMSG_DATA(nlmsg));
    }
    close(sock_fd);
}

void write_to_file(char *str){
    FILE *fp = fopen("results.txt","a");
    if(!fp){
        perror("file");
        exit(-1);
    }
    fprintf(fp, "%s\n", str);
    fclose(fp);
}

void delete_from_file(char *str){
    char* tmpname = "tmp.txt";
    FILE* fp = fopen(filename, "r");
    FILE* outFile = fopen(tmpname, "w+");
    char line [MAX_PAYLOAD]; 
    int lineCount = 0;
    if(!fp){
        printf("Open Error");
    }
    while( fgets(line, sizeof(line), fp) != NULL ){
        char backup[MAX_PAYLOAD];
        strcpy(backup,line);
        printf("%s\n",backup);
        char*token=strtok(line,":");
        printf("%s %s\n", token,str);
        if(strcmp(line,str)){
            fprintf(outFile, "%s", backup);
        }

        lineCount++;
    }
    fclose(fp);
    fclose(outFile);
    remove(filename);
    if( !rename(tmpname, filename) ) {
        printf("Rename Error");
    }
}

