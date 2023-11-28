#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

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
#include <net/ip.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/route.h>
#include <linux/in_route.h>
#include <linux/if_ether.h>
#include "utils.h"


struct packet_queue_head{
    int len;
    int maxlen;
    spinlock_t lock;
    struct list_head list;
};

struct packet_queue_element{
    struct list_head list;
    struct sk_buff *myskb;
    u_int32_t src_ip;
    u_int32_t dst_ip;
    int type;

    //MAC info
    unsigned char mac_dst_addr[ETH_ALEN];

    struct net_device *mydev;
    unsigned char protocol;
};


int init_packet_queue(void);
void clear_packet_queue(void);
int receive_packet(struct sk_buff* skb, int type);
void send_packet(u_int32_t dst_ip, int verdict);
void cleanup_packet_queue(void);
#endif