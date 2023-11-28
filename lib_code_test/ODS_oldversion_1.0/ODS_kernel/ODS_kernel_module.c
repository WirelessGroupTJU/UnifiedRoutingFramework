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

#include "packet_judgment.h"
#include "packet_queue.h"
#include "info_transmission.h"

MODULE_DESCRIPTION("\n ---------ODS kernel module-----------\n This is a official module for my project\n");
MODULE_AUTHOR("YuPeng");
MODULE_LICENSE("GPL");

static int __init init_mymodule(void){
    int judgment_hook_ret,packet_queue_ret,netlink_ret;
    printk(KERN_INFO "============ hello ODS kernel module ============\n");   
    
    judgment_hook_ret = init_judgment_hook();       //register hook function
    if(judgment_hook_ret)
        return 1;
    
    packet_queue_ret = init_packet_queue();         //register packet queue handler
    if(packet_queue_ret)
        return 1;

    netlink_ret = init_netlink();                   //register netlink
    if(netlink_ret)
        return 1;

    return 0;
}

static void __exit exit_mymodule(void){
    clear_netlink();
    clear_packet_queue();
    clear_judgment_hook();
    printk(KERN_INFO "============ goodbye ODS kernel module ============\n");
    return;
}

module_init(init_mymodule);
module_exit(exit_mymodule);

