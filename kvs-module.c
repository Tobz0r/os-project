#include "kvs-module.h"

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
    char recvs[MAX_PAYLOAD]= " ";
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
        char *keyS=strsep(&sepCopy, " ");
        char *valueS=strsep(&sepCopy, " ");
        char *key=kmalloc(sizeof(keyS),GFP_KERNEL);
        char *value=kmalloc(sizeof(valueS),GFP_KERNEL);
        strcpy(key,keyS);
        strcpy(value,valueS);
        printk(KERN_INFO "tok %s  ",token );
        printk(KERN_INFO "key %s  ",key);
        printk(KERN_INFO "val %s  ",value);
        spin_lock(&kvs_lock);
        create_set(store,key,value);
        spin_unlock(&kvs_lock);

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
            strcpy(msg,"Removed entry !");
        }
        //remove entrty ob
    }
    else if(type==PRINT){ 
        //get entrty and add to msg
        if(!store){
            printk(KERN_INFO "eltox vafan");
        }
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
            strcpy(msg,"Something went wrong");
            printk(KERN_INFO "Value is null");
        }else{
            strcpy(msg,KVSvalue->value);
            printk(KERN_INFO "%s", (char*)KVSvalue->value);
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

static int kvs_init(void){
    spin_lock_init(&kvs_lock);

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
    //KVSset *value;
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
 /*   if(!store){
        store = create_store();
    }
    create_set(store,"hello","goodbye");
    create_set(store,"hello2","goodbye2");
    create_set(store,"hello3","goodbye3");
    create_set(store,"hello4","goodbye4");
    create_set(store,"hello5","goodbye5");
    value = get_value(store,"hello3");
    if(value){
    printk(KERN_INFO "%s",(char*)value->value);
	}
    printk(KERN_INFO "ELTOX WAS THE SOLUTION\n");
    proc_parent=create_dir(dirname);
    proc_create("child",0,proc_parent,&kvs_proc_fops);
    proc_create("child2",0,proc_parent,&kvs_proc_fops);*/



    return 0;

}

static void resize_pair(KVSstore *store){
    if(!store){
        return;
    }
    store->set=krealloc(store->set,sizeof(KVSset)*store->nrofelements,GFP_ATOMIC);
}

static void kvs_exit(void)
{
    sock_release(nl_sk->sk_socket);
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
        printk(KERN_INFO "ELTOX WAS HERE \n");
        kfree(tempset->value);
        tempset->value=kmalloc(sizeof(KVSset),GFP_KERNEL);
        tempset->value=value;
    }
    else {
        //create set
        //öka nrofelements
        ++store->nrofelements;
        resize_pair(store);
        newset = &store->set[store->nrofelements -1];
   /*     newset->key=kmalloc(sizeof(*key),GFP_KERNEL);
        memcpy(newset->key,key,sizeof(*key));
        newset->value =kmalloc(sizeof(*value),GFP_KERNEL);
        memcpy(newset->value,value,sizeof(*value));*/
        newset->key=key;
        newset->value=value;

        //sorta med nya värdet
        kvs_sort(store);

    }

}

/* create a new store */

static KVSstore *create_store(void){
    store = kmalloc(sizeof(KVSstore),GFP_KERNEL);
    store->set = kmalloc(sizeof(KVSset),GFP_KERNEL);
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
    if(!store || !store->set){
        printk(KERN_INFO "ELTOX WAS HERE TOO \n");
        return NULL;
    }
    KVSset *tempset = NULL;
    //tempset = bsearch(key,store->set,store->nrofelements,sizeof(KVSset),search_compare);
    return searchKey(key);

}

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
	const KVSset *set=(const KVSset*)element;
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
/*Cortesy of Tom_HellQ */
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
    printk(KERN_INFO "%s ",(char*)key);
    store->nrofelements--;
    resize_pair(store);
    kvs_sort(store);
    return 0;
}
module_init(kvs_init);
module_exit(kvs_exit);


