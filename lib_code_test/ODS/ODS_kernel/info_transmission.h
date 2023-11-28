#ifndef INFO_TRANSMISSION_H
#define INFO_TRANSMISSION_H

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/notifier.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/spinlock.h>
#include <linux/rtnetlink.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <net/netfilter/nf_queue.h>
#include <net/dst.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/string.h>
#include <linux/if_ether.h>

#define NETLINK_TEST 25
#define NLMSG_TYPE_INIT                 0
#define NLMSG_TYPE_INFO                 1
#define NLMSG_TYPE_EXIT                 2
#define IP_FOUND 1
#define IP_NOFOUND 0

struct rt_info{
    //about IP header
    u_int32_t dst_ip;
    u_int32_t src_ip;
    unsigned char protocol;

    //about MAC header
    unsigned char mac_dst_addr[ETH_ALEN];
};

struct rt_result{
    u_int32_t now_ip;
    int result;
};

int init_netlink(void);
void clear_netlink(void);
bool get_transmission_flag(void);
int send_rtinfo(struct rt_info *now_info);


#endif