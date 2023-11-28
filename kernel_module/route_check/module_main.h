#ifndef MODULE_H
#define MODULE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ctype.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/in_route.h>
#include <linux/net.h>
#include <linux/skbuff.h>



// time in secs after which an entry in the route cache is considered
// old and can be deleted just to keep the size of the route_cache under 
//control
#define EXPIRE_TIME 30  // unit is seconds

/* called when a packet is received at the netfilter hook */
unsigned int input_handler(    void *hooknum,
                               struct sk_buff *skb,
                               const struct nf_hook_state *test);




#endif













