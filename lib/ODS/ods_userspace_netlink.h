#ifndef ODS_MODULE
#define ODS_MODULE

#include <linux/netlink.h>
#include <linux/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/netfilter.h>
#include <linux/if_tun.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <net/route.h>
#include <time.h>
#include <ifaddrs.h>  

using namespace std;

#define NETLINK_TEST 25
#define MAX_PAYLOAD 1024
#define NLMSG_TYPE_INIT 0
#define NLMSG_TYPE_INFO 1
#define NLMSG_TYPE_EXIT 2
#define IP_FOUND 1
#define IP_NOFOUND 0
#define PACKET_FROM_LOCAL 0
#define PACKET_FROM_OTHER 1

#define MAC_ADDR_LEN 6

typedef unsigned long addr_t;
typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

// char work_path[1024];

#define TUN_DEV_IP "127.0.0.2"	/* ip address for the tun device */
#define TUN_DEV_NAME "mytun"   /* name of the tun interface */

struct rt_info{
    //about IP header
    u_int32_t dst_ip;
    u_int32_t src_ip;
    unsigned char protocol;

    //about MAC header
    int packet_source_type;
    unsigned char mac_dst_addr[MAC_ADDR_LEN];
};

struct rt_result{
    u_int32_t now_ip;
    int result;
};

struct route_request 
{	
	u_int32_t dst_ip;

	u_int32_t src_ip; 

	unsigned char protocol;
	
	int input_ifindex;
};

struct route_reply{
    u_int32_t now_ip;
    int result;
};

int ods_turn_up(void);
void ods_turn_down(void);

static int ods_init_tun_dev(void);
static int  ods_init_user_socket(void);
static void ods_close_user_socket(void);
static void ods_send_info_msg(addr_t dst_ip, int result);
static void ods_send_init_msg(void);
static void ods_send_exit_msg(void);
static void ods_recv_msg(int fd, struct rt_info* now_info);

int ODS_get_route_request(route_request* now_request);
int ODS_feedback_route_reply(route_reply* now_reply);

#endif