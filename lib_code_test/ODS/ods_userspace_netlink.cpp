#include "ods_userspace_netlink.h"

static int nlfd;
static int tunfd;
static struct sockaddr_nl src, dst;
static struct msghdr msg_send,msg_recv;

static int ods_init_user_socket(void){
    // 创建netlink套接字
    nlfd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_TEST);
    if (nlfd == -1) {
        return 0;
    }
    
    memset(&src, 0, sizeof(struct sockaddr_nl));
    memset(&dst, 0, sizeof(struct sockaddr_nl));
    memset(&msg_send, 0, sizeof(struct msghdr));
    memset(&msg_recv, 0, sizeof(struct msghdr)); 
 
    // 设置本地地址
    src.nl_family = AF_NETLINK; 
    src.nl_pid = getpid(); 
    src.nl_groups = 0;  
 
    // 设置内核netlink地址
    dst.nl_family = AF_NETLINK;
    dst.nl_pid = 0;    
    dst.nl_groups = 0;
 
    // 绑定本地地址
    bind(nlfd, (struct sockaddr*)&src, sizeof(struct sockaddr_nl));

    return nlfd;
}

void ods_close_user_socket(void){
    close(nlfd);
}

static void ods_send_init_msg(void){
    struct iovec iov;
    // 申请netlink消息头域
    struct nlmsghdr *nlh = NULL;
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    // fill header
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = NLMSG_TYPE_INIT;
    // fill payload
    strcpy((char *)NLMSG_DATA(nlh), "Hello you, my kernel moudle!");

    //send message
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg_send.msg_name = (void *)&dst;
    msg_send.msg_namelen = sizeof(dst);
    msg_send.msg_iov = &iov;
    msg_send.msg_iovlen = 1;
    sendmsg(nlfd, &msg_send, 0);
}

static void ods_send_info_msg(addr_t dest_ip, int result){
    struct iovec iov;
    struct rt_result now_result;
    // 申请netlink消息头域
    struct nlmsghdr *nlh = NULL;
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    // fill header
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = NLMSG_TYPE_INFO;
    // fill payload
    now_result.now_ip = (u_int32_t)dest_ip;
    now_result.result = result;
    memcpy(NLMSG_DATA(nlh),&now_result, sizeof(struct rt_result));

    //send message
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg_send.msg_name = (void *)&dst;
    msg_send.msg_namelen = sizeof(dst);
    msg_send.msg_iov = &iov;
    msg_send.msg_iovlen = 1;
    sendmsg(nlfd, &msg_send, 0);
}

static void ods_send_exit_msg(void){
    struct iovec iov;
    // 申请netlink消息头域
    struct nlmsghdr *nlh = NULL;
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    // fill header
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid(); /* self pid */
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = NLMSG_TYPE_EXIT;
    // fill payload
    strcpy((char *)NLMSG_DATA(nlh), "Userspace netlink is going to be exit soon");

    //send message
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg_send.msg_name = (void *)&dst;
    msg_send.msg_namelen = sizeof(dst);
    msg_send.msg_iov = &iov;
    msg_send.msg_iovlen = 1;
    sendmsg(nlfd, &msg_send, 0);
}

static void ods_recv_msg(int fd, struct rt_info *now_info){
    struct iovec iov;
    // 申请netlink消息头域
    struct nlmsghdr *nlh = NULL;
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    // bind
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg_recv.msg_iov = &iov;
    msg_recv.msg_iovlen = 1;
    //recv
    int recv_len = recvmsg(fd, &msg_recv, 0);
    //explain
    memcpy(now_info, NLMSG_DATA(nlh), sizeof(struct rt_info));
    //DEBUG
}   

static int ods_init_tun_dev(void){
    struct ifreq ifr;
    int fd, r;
    char cmd[256];
    unsigned char dev[IFNAMSIZ];
    
    // change to root --vikas
    if(setreuid(0, 0) < 0){
        fprintf(stderr, "setreuid to root failed : \n Please run as root.. ");
        return(-1);
    }

    //   /* FIXME : hardcoded dir name and perms */
    //   umask(0);
    //   mkdir("/dev/net", 0755);

    //   /* MISC_MAJOR and TUN_MINOR char nodes */
    //   mk_node("/dev/net/tun", 10, 200);

    //   // FIXME : should I do a manual insmod here : kmod should do automatically
    
    if ( (fd = open("/dev/net/tun", O_RDWR)) < 0 ) {
        fprintf(stderr, "tun_init(): Error openeing tun device :\n May be you have not configured tun device \n See /usr/src/linux/Documentation/networking/tuntap.txt\n");
        perror("Opening : /dev/net/tun");
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
    *        IFF_TAP   - TAP device  
    *
    *        IFF_NO_PI - Do not provide packet information  
    */ 
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI; 

    strcpy(ifr.ifr_name, TUN_DEV_NAME);

    if ((r = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        perror("ioctl(TUNSETIFF) failed");
            close(fd);
            return -1;
    }
    strcpy((char *)dev, ifr.ifr_name);

    /* FIXME : Is there another way instead of using system */
    /* turn up the interface
    * Is it worth it ?? -vikas */
    sprintf(cmd, "/sbin/ifconfig %s %s netmask 255.255.255.255 up",
        dev, TUN_DEV_IP);
    system(cmd);
        

    #ifdef DEBUG
        fprintf(stderr, "tun_init() : device %s is now up\n", dev);
    #endif
        
    /* no checksum */
    if ((r = ioctl(fd, TUNSETNOCSUM, 1)) < 0) {
        perror("ioctl(TUNSETNOCSUM) failed");
        close(fd);
        return -1;
    }

    return (fd);
}

int ods_turn_up(void){
    if (getcwd(work_path, sizeof(work_path)) == NULL){
        printf("[ODS] get work path failed!\n");
        return -1;
    }
    #ifdef DEBUG
        printf("work path is %s\n", work_path);
    #endif

    // step 1: build a tun dev
    int tun_fd = ods_init_tun_dev();
    if(tun_fd < 0){
        printf("[ODS] tun dev set up failed!\n");
        return -1;
    }
    tunfd = tun_fd;

    // // step 2: add a default route to tun
    // system("route add default gw 192.168.16.3 dev mytun metric 10");

    // step 3: build userspace netlink
    int netlink_ret = ods_init_user_socket();
    if(netlink_ret < 0){
        printf("[ODS] userspace netlink build failed!\n");
        system("rmmod ods_kernel_module.ko");
        return -1;
    }

    // step 4: send a INIT message
    ods_send_init_msg();

    return nlfd;
} 

void ods_turn_down(){
    ods_send_exit_msg();
    ods_close_user_socket();
    exit(1);
}

//  ODS kernel momdule send a route request, this function get the information
int ODS_get_route_request(route_request* now_request){
    if(now_request == NULL){
        printf("[ODS] no space allocated!\n");
        return -1;
    }

    struct rt_info *now_info;
    now_info = (struct rt_info*)malloc(sizeof(struct rt_info));
    if(now_info == NULL){
        printf("[ODS] allocate temp rt_info failed!\n");
        return -1;
    }

    ods_recv_msg(nlfd, now_info);

    now_request->dst_ip = now_info->dst_ip;
    now_request->src_ip = now_info->src_ip;
    now_request->protocol = now_info->protocol;
    
    //get interface index from mac address

    return 0;
}

//  route daemon get the dst ip result, this function send the information to kernel module
int ODS_feedback_route_reply(route_reply* now_reply){
    if(now_reply == NULL){
        printf("[ODS] invalid route_reply!\n");
        return -1;
    }

    ods_send_info_msg(now_reply->now_ip, now_reply->result);
    return 0;
}