

#include <linux/module.h>  /* Needed by all kernel modules */
#include <linux/kernel.h>  /* Needed for loglevels (KERN_WARNING, KERN_EMERG, KERN_INFO, etc.) */
#include <linux/slab.h>    /* kmalloc */
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/bsearch.h>
#include <linux/sort.h>
#include <linux/seq_file.h>
#include <linux/socket.h>
#include <linux/string.h>
#include <linux/net.h>
#include <asm/types.h>
#include <linux/linkage.h>
#include <linux/preempt.h>
#include <linux/spinlock.h>
#include <linux/debug_locks.h>
#include <linux/interrupt.h>
#include <linux/export.h>
#include <linux/init.h> 
#include <linux/mm.h> 
#define ADD '1'
#define REMOVE '2'
#define PRINT '3'

#define NETLINK_USER 21
#define MAX_PAYLOAD 1024
struct sock *nl_sk=NULL;
spinlock_t kvs_lock;




typedef struct 
{
    void *key;
    void *value;
} KVSset;

typedef struct 
{
    KVSset *set;
    size_t nrofelements;
}KVSstore;

struct
{
    struct nlmsghdr *nlh;
    int pid;
}msg;




static void resize_pair(KVSstore *store);

static int search_compare(const void *key, const void *element);

static void kvs_recv_msg(struct sk_buff *skb);

static int kvs_remove_set(KVSstore* store, void* key);

static KVSset *searchKey(void *key);

static KVSstore *create_store(void); 

static void create_set(KVSstore *store, void *key, void *value);

static KVSset *get_value(KVSstore *store, void *key); 
 
/* proc stuff */
//skapar en vanlig fil med namnet "name" file mode "mode" i dir parent

static int compare_funk(const void *x, const void *x2);

static void kvs_sort(KVSstore *store);

static KVSset *get_set(KVSstore *store, const void *key);

//void fileread(const char * filename)
//void filewrite(char* filename, char* data)