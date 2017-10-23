#include "kvs-module.h"
static void resize_pair(KVSstore *store);
static int search_compare(const void *key, const void *element);
static void kvs_recv_msg(struct sk_buff *skb);
static void kvs_remove_set(KVSstore* store, void* key);
//directory
static struct proc_dir_entry *proc_parent;
static char *dirname = "kvs";
static KVSstore *store = NULL;
#define store_size 1024

static void kvs_recv_msg(struct sk_buff *skb){
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char msg[MAX_PAYLOAD];
    int res;
    

    nlh = (struct nlmsghdr *)skb->data;
    char recvs[MAX_PAYLOAD];
    strcpy(recvs,(char*)nlmsg_data(nlh));
    printk(KERN_INFO "Recived msg : %s\n", recvs);
    char type = recvs[0];
    printk(KERN_INFO "type=%c",type);
    if(type==ADD){ //ADD
        strcpy(msg,"Added entry !");
        if(!store){
            store = create_store();
        }
        char *sepCopy=recvs;
        char *token=strsep(&sepCopy, " ");
        char *key=strsep(&sepCopy, " ");
        char *value=strsep(&sepCopy, " ");
        printk(KERN_INFO "tok %s  ",token );
        printk(KERN_INFO "key %s  ",key);
        printk(KERN_INFO "val %s  ",value);
        create_set(store,key,value);

        //add entry
    }
    else if(type==REMOVE){ 
        strcpy(msg,"Removed entry !");
        char *sepCopy=recvs;
        char *token=strsep(&sepCopy, " ");
        char *key=strsep(&sepCopy, " ");
        kvs_remove_set(store,key);
        //remove entrty ob
    }
    else if(type==PRINT){ 
        //get entrty and add to msg
        char *sepCopy=recvs;
        printk(KERN_INFO "1");
        char *token=strsep(&sepCopy, " ");
        printk(KERN_INFO "2%s",token);
        char *key=strsep(&sepCopy, " ");
        printk(KERN_INFO "3%s",key);
        KVSset *value= NULL;
        value=get_value(store,key);
        printk(KERN_INFO "4");
        if(!value){
            printk(KERN_INFO "ditt fel eltox");
            return;
        }else{
            printk(KERN_INFO "%s", value->value);
        }
        strcpy(msg,value->value);
        printk(KERN_INFO "5");

    }
    else{
        strcpy(msg,"Invalid message type")
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

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_ERR "Could not send back msg\n");

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
    KVSset *value;
    /*check if there is any data to collect else send message saying no data         can     be found */
    /* if data exists put data in KVS */
    //int error = 1

        //newstore->set->key = "1";
        //newstore->set->value = "2";
        //delete_value(newstore,newstore->set);    
    //} 
    //else
    //{
        /*tempo ska va en error ist */
    //    printk(KERN_INFO "noett gick fel \n");
    
    //} 
    if(!store){
        store = create_store();
    }
    create_set(store,"hello","goodbye");
    create_set(store,"hello2","goodbye2");
    create_set(store,"hello3","goodbye3");
    create_set(store,"hello4","goodbye4");
    create_set(store,"hello5","goodbye5");
    value = get_value(store,"hello3");
    if(value){
    printk(KERN_INFO "%s",value->value);
	}
    printk(KERN_INFO "ELTOX WAS THE SOLUTION\n");
    proc_parent=create_dir(dirname);
    proc_create("child",0,proc_parent,&kvs_proc_fops);
    proc_create("child2",0,proc_parent,&kvs_proc_fops);



    return 0;

}

static void resize_pair(KVSstore *store){
    if(!store){
        return;
    }
    store->set=krealloc(store->set,sizeof(KVSset)*store->nrofelements,GFP_KERNEL);
}

static void kvs_exit(void)
{
    printk(KERN_INFO "GOODBYE JEBANE \n");
    remove_proc_entry("kvs",NULL);
    /* store, remove threads, free memory and remove KVS */
}
/* create new set */
static void create_set(KVSstore *store, void *key, void *value)
{
    KVSset *newset;
    KVSset *tempset = NULL;
    tempset=get_value(store, key);

    //kollar om de finns i storen
    if(tempset) {
        return;
    }
    else {
        //create set
        //öka nrofelements
        store->nrofelements++;
        resize_pair(store);
        newset = &store->set[store->nrofelements -1];
        newset->key = key;
        newset->value =value;
        printk(KERN_INFO "%s %s \n",newset->key,newset->value);
        //sorta med nya värdet
        kvs_sort(store);
        
    }

}

/* create a new store */

static KVSstore *create_store(void){
    store = kmalloc(store_size,GFP_KERNEL);
    store->set = NULL;
    store->nrofelements = 0;
    printk(KERN_INFO "CREATING STORE \n");
    return store;
} 

/* delete value from store */
//static void delete_value(KVSstore *store, KVSset *set)
//{
    //set->key=NULL;
    //set->value=NULL;
//    --store->size;
//    kfree(set);
    
//}

/* what do */
static KVSset *get_value(KVSstore *store, void *key)
{
    KVSset *tempset = NULL;
    tempset = bsearch(key,store->set,store->nrofelements,sizeof(KVSset),search_compare);
    return tempset;

}

/* compare funktion för qsort */
static int compare_funk(const void *x, const void *x2)
{
    const KVSset *set1 = (const KVSset*)x;
    const KVSset *set2 =(const KVSset*)x2;
    if(set1->key > set2->key)
    {
        return -1;
    }
    if(set1->key < set2->key)
    {
        return 1;
    }
    return 0;
}

static int search_compare(const void *key, const void *element){
	const KVSset *set=element;
	if (key > set->key){
		return -1;
	}
	if(key < set->key){
		return 1;
	}
	return 0;
}
		

static struct proc_dir_entry *create_dir(char *name)
{
    static struct proc_dir_entry *temp;
    temp= proc_mkdir(name,NULL);
    if(!temp){
        printk(KERN_INFO "ERROR CREATING STORE \n");
        //makes module error to load
        //return -ENOMEM;
    }
    return temp;
}
static int kvs_open(struct inode *inode, struct file *file)
{
    return single_open(file,kvs_proc_show,NULL);
}
static int kvs_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m,"hello proc!\n");
    return 0;
}

static void kvs_sort(KVSstore *store)
{
    if(!store)
    {
        printk(KERN_INFO "NO STORE FOUND \n");
        return;
    }
    sort(store->set,store->nrofelements,sizeof(KVSset),compare_funk,NULL);

}
static void kvs_remove_set(KVSstore* store, void* key){
    if(!store){
        printk(KERN_INFO "NO STORE OR SET ");
        return;
    }
    KVSset *set=get_value(store,key);
    store->nrofelements--;
    resize_pair(store);
    set->key=NULL;
    set->value=NULL;
    kvs_sort(store);
}
module_init(kvs_init);
module_exit(kvs_exit);


