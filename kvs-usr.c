#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define NETLINK_USER  23

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src, dest;
struct nlmsghdr *nlmsg = NULL;
struct iovec iov;
struct msghdr msg;


int main(int argc, char**argv){
    if(argc<3){
        printf("Action key Value\n");
    }
    char *type=argv[1];
    char *key=argv[2];
    char *value=argv[3];
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
    dest.nl_pid = 0; /* For Linux Kernel */
    dest.nl_groups = 0; /* unicast */

    /* setting up message*/
    nlmsg = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!nlmsg){
        perror("nlmsg");
        exit(-1);
    }
    memset(nlmsg, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlmsg->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlmsg->nlmsg_pid = getpid();
    nlmsg->nlmsg_flags = 0;
    char msgstr[MAX_PAYLOAD];
    strcpy(msgstr,type);
    strcat(msgstr, " ");
    strcat(msgstr,key);
    strcat(msgstr," ");
    strcat(msgstr,value);
    strcpy(NLMSG_DATA(nlmsg), msgstr);

    iov.iov_base = (void *)nlmsg;
    iov.iov_len = nlmsg->nlmsg_len;
    msg.msg_name = (void *)&dest;
    msg.msg_namelen = sizeof(dest);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("skickar till kernel\n");
    sendmsg(sock_fd, &msg, 0);
    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("Received message: %s\n", (char*)NLMSG_DATA(nlmsg));
    close(sock_fd);
}
