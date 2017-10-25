/**File: Kvs-module.c
**Author: Tobias Estefors, Thom Resntröm
**Destriction: A linux kernel module used to store
**data given from user space
*/
#include "kvs-module.h"

//directory
static KVSstore *store = NULL;
#define store_size 1024
/**
*Recvies message from user space via netlink socket
*Returns: Nothing
*/
static void kvs_recv_msg(struct sk_buff *skb){
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char msg[MAX_PAYLOAD]="";
    int res;
    

    nlh = (struct nlmsghdr *)skb->data;
    char recvs[MAX_PAYLOAD]= " ";
    strcpy(recvs,(char*)nlmsg_data(nlh));
    printk(KERN_INFO "Recived msg : %s\n", recvs);
    char type = recvs[0];
    if(type==ADD){ //ADD
                if(!store){
            store = create_store();
        }
        char *sepCopy=recvs;
        char *token=strsep(&sepCopy, " ");
        char *keyS=strsep(&sepCopy, " ");
        char *valueS=strsep(&sepCopy, " ");
        strcpy(msg,keyS);
        strcat(msg,":");
        strcat(msg,valueS);
        msg[strlen(msg)]='\0';
        char *key=kmalloc(sizeof(keyS),GFP_KERNEL);
        char *value=kmalloc(sizeof(valueS),GFP_KERNEL);
        strcpy(key,keyS);
        strcpy(value,valueS);
        spin_lock(&kvs_lock);
        create_set(store,key,value);
        spin_unlock(&kvs_lock);
                printk(KERN_INFO"%s",msg);


        //add entry
    }
    else if(type==REMOVE){ 
        
        char *sepCopy=recvs;
        char *token=strsep(&sepCopy, " ");
        char *key=strsep(&sepCopy, " ");
        char *keySend=kmalloc(sizeof(key),GFP_KERNEL);
        strcpy(keySend,key);
        spin_lock(&kvs_lock);
        int ret=kvs_remove_set(store,keySend);
        spin_unlock(&kvs_lock);
        if(ret!=0){
            strcpy(msg,"Entry does not exist!");
        }else{
            strcpy(msg,key);
        }
        //remove entrty ob
    }
    else if(type==PRINT){ 
        //get entrty and add to msg
        char *sepCopy=recvs;
        char *token=strsep(&sepCopy, " ");
        char *key=strsep(&sepCopy, " ");
        char *keySend=kmalloc(sizeof(key),GFP_KERNEL);
        strcpy(keySend,key);
        KVSset *KVSvalue;
        spin_lock(&kvs_lock);
        KVSvalue=get_value(store,keySend);
        spin_unlock(&kvs_lock);
        if(!KVSvalue){
            strcpy(msg,"Nothing to print");
            printk(KERN_INFO "Value is null");
        }else{
            strcpy(msg,KVSvalue->value);
        }

    }
    else{
        strcpy(msg,"Invalid message type");
        printk(KERN_ERR "Invalid message type\n");
        return ;
    }
    pid = nlh->nlmsg_pid; /*pid of sending process */
    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate socket buffer\n");
        return ;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);
    spin_lock(&kvs_lock);
    res = nlmsg_unicast(nl_sk, skb_out, pid);
    spin_unlock(&kvs_lock);
    if (res < 0)
        printk(KERN_ERR "Could not send back msg\n");

}

/**
*Initiate the module
* Return: 0 if succeded else 10
*/
static int kvs_init(void){
    /* skapa struct för recv funktion*/
    struct netlink_kernel_cfg cfg = {
        .input = kvs_recv_msg,
    };
    /*sätt upp connectiont ill usr-space*/
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;

}

/*
* Resize the set pairs by new size'
* Return: Nothing
*/
static void resize_pair(KVSstore *store){
    if(!store){
        return;
    }
    store->set=krealloc(store->set,sizeof(KVSset)*store->nrofelements,GFP_ATOMIC);
}

static void kvs_exit(void){
    sock_release(nl_sk->sk_socket);
    printk(KERN_INFO "Goodbye world \n");
    kfree(store->set);
    kfree(store);
    /* store, remove threads, free memory and remove KVS */
}

/* create new set */
/* Returns: Nothing*/
static void create_set(KVSstore *store, void *key, void *value){
    KVSset *newset;
    KVSset *tempset = NULL;
    tempset=get_value(store, key);

    //checks if the key exists
    if(tempset) {
        kfree(tempset->value);
        tempset->value=kmalloc(sizeof(KVSset),GFP_KERNEL);
        tempset->value=value;
    }
    else {
        ++store->nrofelements;
        resize_pair(store);
        newset = &store->set[store->nrofelements -1];
        newset->key=key;
        newset->value=value;
        kvs_sort(store);
    }
}

/* create a new store */
/* Returns: A new struct with the store*/
static KVSstore *create_store(void){
    store = kmalloc(sizeof(KVSstore),GFP_KERNEL);
    store->set = kmalloc(sizeof(KVSset),GFP_KERNEL);
    store->nrofelements = 0;
    printk(KERN_INFO "CREATING STORE \n");
    return store;
} 



/**
* Returns the value based on key*/
/* Returns : A set with the value to assisosiated key*/
static KVSset *get_value(KVSstore *store, void *key)
{
    if(!store || !store->set){
        return NULL;
    }
    return searchKey(key);

}

/** Search for the set with the given key**/
static KVSset *searchKey(void *key){
    for(int i = 0 ; i < store->nrofelements;i++){
        KVSset *newset = &store->set[i];
        printk(KERN_INFO "%s \n",(char*)newset->key);
        if(memcmp(newset->key,key,sizeof(newset->key))==0){
            return newset;
        }
    }
    return NULL;

}
static KVSset *get_set(KVSstore *store, const void *key){
    return NULL;
}

/* compare funktion för qsort */
static int compare_funk(const void *x, const void *x2){
    const KVSset *set1 = (const KVSset*)x;
    const KVSset *set2 =(const KVSset*)x2;
    if(set1->key > set2->key){
        return -1;
    }
    if(set1->key < set2->key){
        return 1;
    }
    return 0;
}

		

/** sorts the kvs store, usefull when entry gets removed**/
static void kvs_sort(KVSstore *store){
    if(!store)
    {
        printk(KERN_INFO "NO STORE FOUND \n");
        return;
    }
   sort(store->set,store->nrofelements,sizeof(KVSset),compare_funk,NULL);
}
/** Deletes a set of pairs from the kvs store based on the key**/
static int kvs_remove_set(KVSstore* store, void* key){
    if(!store){
        printk(KERN_INFO "NO STORE OR SET ");
        return 1;
    }
    KVSset *set=get_value(store,key);
    if (!set){
        return 1;
    }
    KVSset* tempset=&store->set[store->nrofelements-1];
    if(tempset->key!=key){
        set->key=tempset->key;
        set->value=tempset->value;
    }
    else{
        set->key=NULL;
        set->value=NULL;
    }
    store->nrofelements--;
    resize_pair(store);
    kvs_sort(store);
    return 0;
}


/*******CODE FOR READING FILE, DOES NOT WORK AS INDENTED*****/
/*
void fileread(const char * filename){
  struct file *filp;
  struct inode *inode;
  mm_segment_t old_fs;
  off_t fsize;
  unsigned long magic;
  filp=filp_open(filename,O_RDONLY,0);
  if(!filp){
    printk(KERN_INFO "Fail\n");
    return;
  }
  inode=file_inode(filp);
  fsize=inode->i_size;
  char buf[fsize+1];
  old_fs=get_fs();
  set_fs(KERNEL_DS);
  filp->f_op->llseek(filp,0,0);
  filp->f_op->read(filp,buf,fsize,&filp->f_pos);
  set_fs(old_fs);
  buf[fsize]='\0';
  //printk("%s",buf);
  filp_close(filp,NULL);
}

void filewrite(char* filename, char* data){
  struct file *filp;
  mm_segment_t old_fs;
  filp = filp_open(filename, O_RDWR|O_CREAT, 0644);
  if(!filp){
    printk(KERN_ERROR"error file");
    return;
  }
  old_fs=get_fs();
  set_fs(KERNEL_DS);
  filp->f_op->write(filp, data, strlen(data),&filp->f_pos);
  set_fs(old_fs);
  filp_close(filp,NULL);
}
*/

module_init(kvs_init);
module_exit(kvs_exit);


