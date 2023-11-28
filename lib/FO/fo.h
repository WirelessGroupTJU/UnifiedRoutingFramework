#ifndef FO
#define FO

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>

using namespace std;

typedef unsigned long addr_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;
typedef unsigned int u_int;

struct route_entry{
    uint32_t dst;
    uint32_t gateway;
    int ifindex;
    int metric;
    uint32_t src;
    int table;
    route_entry* nextptr;
};

//nlmsg request structure
struct rtreq {
  struct nlmsghdr n;
  struct rtmsg r;
  char buf[1024];
};

static int fo_socket_fd;

int init_fo_socket();
void close_fo_socket();
int FO_add_a_route(uint32_t dst, int netmask,uint32_t gateway, int ifindex, int metric, uint32_t src);
int FO_delete_a_route(uint32_t dst, uint32_t gateway, int ifindex, int metric, uint32_t src);
int FO_get_routes(route_entry* table, int* len);

/* return the idle time(in msec) recorded for a destination.
* vFlag argument has following meaning associated with it's values:
* vFlag = 1, retreive idle time since a packet was forwarded last to dest_ip
* vFlag = 0, retreive idle time since a packet was received last from dest_ip
* this uses the route_check module */
int FO_query_route_idle_time(addr_t dest_ip, int vFlag);

#endif