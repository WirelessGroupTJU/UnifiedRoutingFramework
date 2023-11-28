#include "packet_queue.h"
#include "info_transmission.h"

#define MAX_PACKET_NUMBER 1000

static struct packet_queue_head *my_queue_head;
static struct ip_queue_head *my_ip_head;

static int packet_enqueue(struct sk_buff* skb, int type){
    struct packet_queue_element *now_element;
    struct iphdr *iph = ip_hdr(skb);
    
    now_element = kmalloc(sizeof(struct packet_queue_element), GFP_ATOMIC);
    if(now_element == NULL){
        return 1;
    }
    now_element->myskb = skb;
    now_element->dst_ip = iph->daddr;
    now_element->src_ip = iph->saddr;
    now_element->mydev = skb_dst(skb)->dev;
    now_element->protocol = iph->protocol;
    now_element->type = type;

    spin_lock_bh(&my_queue_head->lock);
    if(my_queue_head->len < my_queue_head->maxlen){
        list_add(&now_element->list, &my_queue_head->list);
        my_queue_head->len++;
        spin_unlock_bh(&my_queue_head->lock);
        return 0;
    }else {
        spin_unlock_bh(&my_queue_head->lock);
        kfree(now_element);
        return 1;
    }
}

static struct packet_queue_element *packet_dequeue(int (*cmp)(struct packet_queue_element*, u_int32_t), u_int32_t now_ip){
    struct list_head *my_head;
    spin_lock_bh(&my_queue_head->lock);
    for(my_head = my_queue_head->list.prev; my_head != &my_queue_head->list; my_head = my_head->prev){
        struct packet_queue_element *element = (struct packet_queue_element *)my_head;
        if(!cmp || cmp(element, now_ip)){                                              //no cmp or cmp is true
            list_del(&element->list);
            my_queue_head->len --;
            spin_unlock_bh(&my_queue_head->lock);
            return element;
        }
    }
    spin_unlock_bh(&my_queue_head->lock);
    return NULL;
}

static int judge_ip(struct packet_queue_element *now_entry, u_int32_t now_ip){
    if(now_entry->dst_ip == now_ip){
        return 1;
    }else {
        return 0;
    }
}

static bool pending_ip(u_int32_t now_ip){
    struct list_head *my_head;
    spin_lock_bh(&my_queue_head->lock);
    for(my_head = my_queue_head->list.prev; my_head != &my_queue_head->list; my_head = my_head->prev){
        struct packet_queue_element *element = (struct packet_queue_element *)my_head;
        if(element->dst_ip == now_ip){                                              //no cmp or cmp is true
            spin_unlock_bh(&my_queue_head->lock);
            return true;    //find ip
        }
    }
    spin_unlock_bh(&my_queue_head->lock);
    return false;           //not find ip
}

static void transmit_rtinfo(struct sk_buff* skb){
    int transmit_ret;
    struct rt_info now_info;
    struct iphdr *iph = ip_hdr(skb);

    now_info.src_ip = iph->saddr;
    now_info.dst_ip = iph->daddr;
    now_info.protocol = iph->protocol;

    transmit_ret = send_rtinfo(&now_info);
    if(transmit_ret != 0){
        printk(KERN_INFO "transmit rt_info failed\n");
    }
}

static int reroute(struct sk_buff* myskb, int type)
{   printk(KERN_INFO "reroute enter\n");

    struct iphdr *iph = ip_hdr(myskb);
    if(type == LOCAL){
        struct rtable *rt;
        rt = ip_route_output(dev_net(skb_dst(myskb)->dev), iph->daddr, iph->saddr, RT_TOS(iph->tos), 0);

        /* Drop old route. */
        dst_release(skb_dst(myskb));
        // skb_dst_drop(myskb);
        skb_dst_set(myskb, &(rt->dst));

    }else if(type == OTHER){
        ip_route_input_noref(myskb, iph->daddr, iph->saddr, iph->tos, myskb->dev);
    }
    // NF_HOOK(PF_INET, NF_INET_LOCAL_OUT ,dev_net(skb_dst(myskb)->dev), myskb->sk, myskb, NULL, skb_dst(myskb)->dev, dst_output);
    // ip_output(dev_net(myskb->dev), myskb->sk, myskb);
    // dst_output(NULL, NULL, myskb);
    // ip_local_out(dev_net(myskb->dev), myskb->sk, myskb);
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
// static int receive_packet(struct nf_queue_entry *entry, unsigned int queuenum){
//     int enqueue_ret;
//     struct iphdr *iph=ip_hdr(entry->skb);
//     bool enqueue_flag = false;                                  //enqueue_flag is a secure flag
//     u_int32_t dst_ip = iph->daddr;
//     if(!pending_ip(dst_ip)){                                    //pending return false->not exist return true->exist
//         transmit_rtinfo(entry);
//     }
    
//     // struct ethhdr *ethh = eth_hdr(entry->skb);
//     // printk(KERN_INFO "dst mac addr is %02x:%02x:%02x:%02x:%02x:%02x\n"
//     //                                                 , ethh->h_dest[0]
//     //                                                 , ethh->h_dest[1]
//     //                                                 , ethh->h_dest[2]
//     //                                                 , ethh->h_dest[3]
//     //                                                 , ethh->h_dest[4]
//     //                                                 , ethh->h_dest[5]);

//     enqueue_flag = get_transmission_flag();

//     // #ifdef DEBUG
//     //     printk(KERN_INFO "enqueue flag is %d\n", enqueue_flag);
//     // #endif

//     if(enqueue_flag){
//         enqueue_ret = packet_enqueue(entry);
//         if(enqueue_ret){
//             printk(KERN_INFO "enqueue a new packet element failed\n");
//             nf_reinject(entry, NF_DROP);
//         }

//         #ifdef DEBUG
//             printk(KERN_INFO "[receive_packet] there have %d pakcet\n", my_queue_head->len);
//         #endif
//     }
//     return 0;
// }


int receive_packet(struct sk_buff* skb, int type){
    int enqueue_ret;
    struct iphdr *iph=ip_hdr(skb);
    u_int32_t dst_ip = iph->daddr;

    /*
        now time, if flag is false, that means user netlink have not ok,the packet need to be free
    */
    bool enqueue_flag = false;                                  //enqueue_flag is a secure flag
    enqueue_flag = get_transmission_flag();
    if(enqueue_flag == false){
        kfree_skb(skb);
        return 1;
    }

    
    if(!pending_ip(dst_ip)){                                    //pending return false->not exist return true->exist
        transmit_rtinfo(skb);
    }


    enqueue_ret = packet_enqueue(skb, type);
    if(enqueue_ret){
        printk(KERN_INFO "enqueue a new packet element failed\n");
        kfree_skb(skb);
    }

    #ifdef DEBUG
        printk(KERN_INFO "[receive_packet] there have %d pakcet\n", my_queue_head->len);
    #endif

    return 0;
}

int init_packet_queue(void){
    my_queue_head = kmalloc(sizeof(struct packet_queue_head), GFP_KERNEL);
    if(my_queue_head == NULL){
        printk(KERN_INFO "kallocate my_queue_head failed\n");
        return 1;
    }
    my_queue_head->len = 0;
    my_queue_head->maxlen = MAX_PACKET_NUMBER;
    INIT_LIST_HEAD(&my_queue_head->list);                           //init list
    spin_lock_init(&my_queue_head->lock);                           //init lock

    // nf_register_queue_handler(&nfqh);
    // nf_register_queue_handler(&init_net, &nfqh);
    return 0;
}

void send_packet(u_int32_t dst_ip, int verdict){                                 //release packets whose ip is equal to dst_ip
    struct packet_queue_element *now_entry;
    while((now_entry = packet_dequeue(judge_ip, dst_ip))){                       //now_entry is not NULL
        
        if(verdict == NF_DROP){
            kfree_skb(now_entry->myskb);
            kfree(now_entry);
        }else if(verdict == NF_ACCEPT){
            
            reroute(now_entry->myskb, now_entry->type);
            #ifdef DEBUG
                struct net_device *now_dev = skb_dst(now_entry->myskb)->dev;
                printk(KERN_INFO "reroute interface is: %s\n", now_dev->name);
            #endif
            
            ip_local_out(dev_net((now_entry->myskb)->dev), (now_entry->myskb)->sk, now_entry->myskb);
            // ip_local_out(now_entry->entry->state.net, now_entry->myskb->sk, now_entry->myskb);
            kfree(now_entry);
        }
        
    }
}

void cleanup_packet_queue(void){
    while(my_queue_head->len){
        struct packet_queue_element *entry = packet_dequeue(NULL, 0);
        struct net_device *now_dev = dev_get_by_name(&init_net, SENDDEV);
        skb_dst(entry->myskb)->dev = now_dev;
        kfree_skb(entry->myskb);
        kfree(entry);
        // nf_reinject(entry->entry, NF_DROP);                        //FIXME:kfree entry?
    }
}

void clear_packet_queue(void){
    cleanup_packet_queue();
    #ifdef DEBUG
        printk(KERN_INFO "now have %d packets\n", my_queue_head->len);
    #endif
    kfree(my_queue_head);
    kfree(my_ip_head);
    // nf_unregister_queue_handler(&init_net);
    // nf_unregister_queue_handler();
}