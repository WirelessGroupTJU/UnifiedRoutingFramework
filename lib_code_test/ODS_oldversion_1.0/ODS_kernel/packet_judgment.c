#include "packet_judgment.h"
#include "utils.h"

static struct nf_hook_ops filter_post_routing;
// static struct nf_hook_ops filter_forward;

static bool judgment_packet(struct sk_buff *skb, u_int32_t dst_ip, struct net_device *dst_dev){
    bool judgment_ret = true;
    if(!strcmp(dst_dev->name, TUNNAME)){    //same is 0
        judgment_ret = false;

        struct ethhdr *ethh = eth_hdr(skb);
        struct iphdr *iph=ip_hdr(skb);
        u_int32_t dst_ip=0, src_ip=0;
        dst_ip = iph->daddr;
        src_ip = iph->saddr;

        // printk(KERN_INFO "[judgment_packet] indev is %s\n", (skb->dev)->name);

    }
    return judgment_ret;
}

static unsigned int check_packet(void *hooknum, struct sk_buff *skb, const struct nf_hook_state *test){
    struct iphdr *iph=ip_hdr(skb);
    struct net_device *dst_dev = skb_dst(skb)->dev;
    u_int32_t dst_ip=0;
    dst_ip = iph->daddr;
    
    if(judgment_packet(skb, dst_ip, dst_dev)){
        // printk(KERN_INFO "[check packet] dst_ip: %s\n", inet_ntoa(dst_ip));
        return NF_ACCEPT;
    }else {
        return NF_QUEUE;
    }
}

static void init_hook_filter(void){
    filter_post_routing.hook = check_packet;
    filter_post_routing.pf = PF_INET;
    filter_post_routing.hooknum = NF_INET_POST_ROUTING;

    // filter_forward.hook = check_packet;
    // filter_forward.pf = PF_INET;
    // filter_forward.hooknum = NF_INET_FORWARD;
}


int init_judgment_hook(void){
    int ret_postrouting, ret_forward;
    init_hook_filter();

    // ret_forward = nf_register_net_hook(&init_net,&filter_forward);
    // if(ret_forward){
    //     printk(KERN_INFO "route_check: error registering NF_FORWARD\n");
    // }

    ret_postrouting = nf_register_net_hook(&init_net,&filter_post_routing);
    if(ret_postrouting){
        printk(KERN_INFO "route_check: error registering NF_POSTROUTING hook\n");
    }

    if(!ret_forward && !ret_postrouting) return 0;
    else return 1;
}

void clear_judgment_hook(void){
    // nf_unregister_net_hook(&init_net,&filter_forward);
    nf_unregister_net_hook(&init_net,&filter_post_routing);
}