
#include "module-kvs.h"

static struct proc_dir_entry *proc_parent;
static struct proc_dir_entry *proc_child;

static void kvs_recv_msg(struct sk_buff *skb){
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg;
    int res;

    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:%s\n", (char *)nlmsg_data(nlh));
    char typec = (char*)nlmsg_data(nlh)[0];
    int type=typec;
    if(type==ADD){
        msg="Added entry!";
    }
    else if(type==REMOVE){
        msg="Removed entry"
    }
    else if(type==PRINT){
        //do stuff
        msg="....";
    }
    else{
        printk(KERN_ERR "Invalid message type\n");
        return -1;
    }
    pid = nlh->nlmsg_pid; /*pid of sending process */
    msg_size = strlen(msg);
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate socket buffer\n");
        return -1;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Eltox plz no\n");

}

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
    //skapa parent
    proc_parent = proc_mkdir("store",NULL);
    if(!proc_parent){
        printk(KERN_INFO "why you do this eltosic \n");
    }
    proc_child =proc_create("child",0,proc_parent,&kvs_proc_fops);
    printk(KERN_INFO "GOD TOX \n");
    return 0;
    printk(KERN_INFO "yeeeboooi \n");
}

static void kvs_exit(void){
    printk(KERN_INFO "GOODBYE JEBANE \n");
    remove_proc_entry("kvs",NULL);
    /* store, remove threads, free memory and remove KVS */
}
/* create new set */
static void *create_set(KVSstore *store, void *key, void *value){
    KVSset *newset;
    KVSset *tempset = get_value(store, key);
    newset->key = key;
    newset->value =value;
    return newset;
}

/* create a new store */
static KVSstore *create_store(void){
    //KVSstore *store;
    //store = kmalloc(sizeof(KVSstore),GFP_KERNEL);
    //store->set = NULL;
    //store->size = 0;
    //printk(KERN_INFO "CREATING STORE \n");
    return NULL;
}

/* delete value from store */
static void delete_value(KVSstore *store, KVSset *set){
    //set->key=NULL;
    //set->value=NULL;
    --store->size;
    kfree(set);   
}
/* create new set */
static void set_value(KVSstore *store, KVSset *set)
{

    //kanske samma som create_set

}
/* what do */
static KVSset *get_value(KVSstore *store, void *key)
{
    KVSset *tempset = NULL;
    //tempset = 
    return tempset;


}


static int kvs_open(struct inode *inode, struct file *file){
    return single_open(file,kvs_proc_show,NULL);
}
static int kvs_proc_show(struct seq_file *m, void *v){
    seq_printf(m,"hello proc!\n");
    return 0;
}


module_init(kvs_init);
module_exit(kvs_exit);