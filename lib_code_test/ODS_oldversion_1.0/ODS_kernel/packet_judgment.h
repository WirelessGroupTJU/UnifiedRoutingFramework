#ifndef PACKET_JUDGMENT
#define PACKET_JUDGMENT

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
#include <linux/string.h>

#define TUNNAME "mytun"

int init_judgment_hook(void);
void clear_judgment_hook(void);

#endif