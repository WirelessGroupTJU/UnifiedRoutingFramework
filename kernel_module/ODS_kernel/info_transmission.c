#include "info_transmission.h"
#include "packet_queue.h"


static bool transmission_flag = false;
static int user_pid = -1;
static struct sock *socket_of_netlink = NULL;
static struct netlink_kernel_cfg cfg;

static void deal_init_msg(struct sk_buff  *received_skb){       //get user id
    struct nlmsghdr *nlmessage;
    nlmessage = nlmsg_hdr(received_skb);
    user_pid = nlmessage->nlmsg_pid;
    transmission_flag = true;
    printk(KERN_INFO "init netlink cfg: user_id is %d, contain is %s\n", user_pid, (char *)NLMSG_DATA(nlmessage));
    return;
}

static void deal_info_msg(struct sk_buff  *received_skb){       //get asl result
    struct nlmsghdr *nlmessage;
    struct rt_result *now_result;
    int result;
    u_int32_t now_ip;
    nlmessage = nlmsg_hdr(received_skb);
    now_result = (struct rt_result*)NLMSG_DATA(nlmessage);
    now_ip = now_result->now_ip;
    result = now_result->result;
    printk(KERN_INFO "rt_result ip is %s result is %d\n", inet_ntoa(now_result->now_ip), now_result->result);
    if(now_result->result == IP_FOUND){
        send_packet(now_result->now_ip, NF_ACCEPT);
    }else {
        send_packet(now_result->now_ip, NF_DROP);
    }
    return;
}

static void deal_exit_msg(struct sk_buff  *received_skb){
    struct nlmsghdr *nlmessage;
    nlmessage = nlmsg_hdr(received_skb);
    user_pid = nlmessage->nlmsg_pid;
    transmission_flag = false;
    printk(KERN_INFO "init netlink cfg: user_id is %d, contain is %s\n", user_pid, (char *)NLMSG_DATA(nlmessage));
    return;
}

static void recv_msg(struct sk_buff  *received_skb){            
    struct nlmsghdr *nlmessage;
    nlmessage = nlmsg_hdr(received_skb);

    switch(nlmessage->nlmsg_type){
        case NLMSG_TYPE_INIT :
            printk(KERN_INFO "msg is init\n");
            deal_init_msg(received_skb);
            break;
        case NLMSG_TYPE_INFO :
            printk(KERN_INFO "msg is info\n");
            deal_info_msg(received_skb);
            break;
        case NLMSG_TYPE_EXIT :
            printk(KERN_INFO "msg is that userspace netlink will be exit\n");
            deal_exit_msg(received_skb);
            break;
        default:
            printk(KERN_INFO "msg is unknown\n");
            break;
    }
    return;
}

int init_netlink(void){
    cfg.input = recv_msg;
    socket_of_netlink = netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    if(socket_of_netlink == NULL){
        printk(KERN_INFO "allocate a netlink in kernel failed!\n");
        return 1;
    }
    return 0;
}

int send_rtinfo(struct rt_info *now_info){
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh = NULL;
    int datalen;
    struct rt_info *head;
    datalen = NLMSG_SPACE(sizeof(struct rt_info));
 
    skb = alloc_skb(datalen, GFP_KERNEL);
    if(!skb)
    {
        printk(KERN_INFO "rt_info allocate failed\n");
        return -1;
    }
    // 数据初始化
    nlh = nlmsg_put(skb, 0, 0, 0, sizeof(struct rt_info), 0);
    nlh->nlmsg_pid = 0;
    nlh->nlmsg_type = NLMSG_TYPE_INFO;

    // #ifdef DEBUG
    //     printk(KERN_INFO "send_rtinfo: dst_ip=%s\n", inet_ntoa(now_info->dst_ip));
    // #endif
    
    head = (struct rt_info*)NLMSG_DATA(nlh);
    memcpy(head, now_info, sizeof(struct rt_info));
    netlink_unicast(socket_of_netlink, skb, user_pid, MSG_DONTWAIT);
    return 0;    
}

bool get_transmission_flag(void){
    return transmission_flag;
}

void clear_netlink(void){
    if(socket_of_netlink){
        netlink_kernel_release(socket_of_netlink);
        socket_of_netlink = NULL;
    }
}