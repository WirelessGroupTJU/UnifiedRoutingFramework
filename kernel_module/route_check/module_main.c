/* Code for the route_check module of the Ad hoc Support Framework
* This implements a module which registers with the netfilter's
* POST_ROUTING hook and record a timestamp when the route was last used
* before re-injecting the packet.
* This statistics is then available in /proc/asl/route_check directory
* for use directly by the routing daemons or by the adhoc support daemon 
* framework.
* 
* Vikas Kawadia, May 19,2002 */

#include "module_main.h"
#include "route_cache.h"
#include "utils.h"

#define DEBUG9

//extern struct route_cache_entry *route_cache;

/* entries for our different PROC files */
/* /proc/asl/route_check */
static struct proc_dir_entry *asl_dir, *route_check;

/* for the netfilter hooks */
struct nf_hook_ops filter;

MODULE_DESCRIPTION("\n -------- route_check.o -----------\n route_check module implements support for recoding the time of previous use of a route in the kernel routing table. The information is output in /proc/asl/route_check. \n This information can be used by user space routing daemons in deleteing stale enries. This is a part of the Ad hoc Support Library. For details see http://aslib.sourceforge.net \n");

MODULE_AUTHOR("Vikas Kawadia");
MODULE_LICENSE("GPL");
/****************************************************
	init_module()
----------------------------------------------------
Called by insmod at startup.
****************************************************/


const struct proc_ops  proc_ops = {
	.proc_read = read_route_cache_proc,
	

};


int init_module(void)
{
    int result;

    printk(KERN_INFO "Adhoc Support Framework : Route_check module: Starting up\n");

    /* netfilter stuff
	* This modules registers at the post routing hook */

    filter.hook = input_handler;
    filter.pf = PF_INET; // IPv4
    filter.hooknum = 4;

    /* hook registration */
    result = nf_register_net_hook(&init_net,&filter);
    if (result){
    	printk(KERN_INFO "route_check: error registering hook (%d)", result);
    	return 1;
	}
#ifdef DEBUG9
    printk(KERN_NOTICE "route_check: input filter registered!\n");
#endif

    //setup the proc file system
    asl_dir=proc_mkdir("asl",NULL);
    route_check=proc_create_data("asl/route_check", 0, NULL, &proc_ops, NULL);
    //route_check->owner=THIS_MODULE;

#ifdef DEBUG9
    printk(KERN_NOTICE "route_check: proc entry created successfully !\n");
#endif

    return 0;

}


/****************************************************

   cleanup_module
----------------------------------------------------
cleans up the module. called by rmmod
****************************************************/
void cleanup_module(void)
{
    //detlet the proc files
    remove_proc_entry("asl/route_check",NULL);
    remove_proc_entry("asl", NULL);

    /* unregister hooks */
    nf_unregister_net_hook(&init_net,&filter);

#ifdef DEBUG9
    printk(KERN_NOTICE "Route_check : Unregistered NetFilter hooks... Exiting...\n");
#endif
}


/****************************************************
   input_handler()
----------------------------------------------------
The function which gets called by netfilter when 
a packet is received 
****************************************************/
unsigned int input_handler(void *hooknum, struct sk_buff *skb, const struct nf_hook_state *test )
{
	struct iphdr *iph=ip_hdr(skb);
	struct route_cache_entry *tmp_entry=NULL ;
	u_int32_t dst_ip=0, src_ip=0;
	u_int64_t last_use_time=0 ;
	int			vFlag=1;

#ifdef DEBUG9
		printk(KERN_NOTICE "Inside Input_handler\n");
#endif

	if (udp_hdr(skb)!=NULL) {

		dst_ip = iph->daddr ;
		src_ip = iph->saddr ;
		last_use_time = getcurrtime();
#ifdef DEBUG9
		printk(KERN_INFO "Input_handler:src_ip=%s,  dst_ip= %s , last_use_time =%llu\n", inet_ntoa(src_ip),inet_ntoa(dst_ip),last_use_time);
#endif		

		/* check for destination entry */
		tmp_entry = find_route_cache_entry(dst_ip, vFlag);
		if(tmp_entry == NULL){
			// if not found insert the entry
			insert_route_cache_entry(dst_ip, last_use_time, vFlag);
		} else {
			//else update the entry
			tmp_entry->last_use_time = last_use_time ;
		}
			
		/* check for source entry, set vFlag to 0 for source ip */
		vFlag = 0;
		tmp_entry = find_route_cache_entry(src_ip, vFlag);
		if(tmp_entry == NULL){
			// if not found insert the entry
			insert_route_cache_entry(src_ip, last_use_time, vFlag);
		} else {
			//else update the entry
			tmp_entry->last_use_time = last_use_time ;
		}
			

    }
    return NF_ACCEPT;

}





