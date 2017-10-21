#include <sys/socket.h>  
#include <linux/netlink.h>  
#define NETLINK_USER 31  
#define MAX_PAYLOAD 1024  /* maximum payload size*/  

#define ADD 1
#define REMOVE 2
#define PRINT 3