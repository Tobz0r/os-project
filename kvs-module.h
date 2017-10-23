

#include <linux/module.h>  /* Needed by all kernel modules */
#include <linux/kernel.h>  /* Needed for loglevels (KERN_WARNING, KERN_EMERG, KERN_INFO, etc.) */
#include <linux/init.h>    /* Needed for __init and __exit macros. */
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
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/socket.h>
#include <linux/string.h>
#include <linux/net.h>
#include <asm/types.h>
#define ADD '1'
#define REMOVE '2'
#define PRINT '3'

#define NETLINK_USER 23
#define MAX_PAYLOAD 1024
struct sock *nl_sk=NULL;

//måste va ovanför för jebane
static int kvs_open(struct inode *inode, struct file *file);

struct file_operations kvs_proc_fops ={
    .owner = THIS_MODULE,
    .open =kvs_open,
    .read =seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

typedef struct 
{
    void *key;
    void *value;
} KVSset;

typedef struct 
{
    KVSset *set;
    int nrofelements;
}KVSstore;

struct
{
    struct nlmsghdr *nlh;
    int pid;
}msg;


static KVSstore *create_store(void); //create store

static void create_set(KVSstore *store, void *key, void *value); //create set

//static void set_value(KVSstore *store, KVSset *set); //store the "value" in memory under indentifier "key, so we can access it later. if some data was already presented under "key, this data will be replaced.

static KVSset *get_value(KVSstore *store, void *key); //get some data previously saved under the identifier "key", or fail if no data was stored for "key"
 
//static void delete_value(KVSstore *store, KVSset *set); //delete the data that was stored under "key"

/* proc stuff */
//skapar en vanlig fil med namnet "name" file mode "mode" i dir parent

static int compare_funk(const void *x, const void *x2);
static struct proc_dir_entry *create_dir(char *name);
static int kvs_proc_show(struct seq_file *m, void *v);
static void kvs_sort(KVSstore *store);

//skapa en symlink (symbolisk link)

//skapa en directory



