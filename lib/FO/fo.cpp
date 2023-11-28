#include "./fo.h"

/*
    current version is for IPv4, so some type is also suit for IPv4
    such as
        family
        family_size
*/
unsigned char family = AF_INET;
uint32_t rttable = RT_TABLE_MAIN;
unsigned int flags = RTNH_F_ONLINK;
unsigned char scope = RT_SCOPE_UNIVERSE;
int protocol = RTPROT_STATIC;
int family_size = sizeof(struct in_addr);


static void
netlink_addreq(struct nlmsghdr *n, size_t reqSize __attribute__ ((unused)), int type, const void *data, int len)
{
  struct rtattr *rta = (struct rtattr *)(void*)(((char *)n) + NLMSG_ALIGN(n->nlmsg_len));
  n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_LENGTH(len);
  rta->rta_type = type;
  rta->rta_len = RTA_LENGTH(len);
  memcpy(RTA_DATA(rta), data, len);
}

static int
netlink_send_update_request(int socketfd, struct nlmsghdr *nl_hdr)
{
  char rcvbuf[1024];
  struct iovec iov;
  struct sockaddr_nl nladdr;
  struct msghdr msg;
  struct nlmsghdr *h;
  struct nlmsgerr *l_err;
  int ret;

  memset(&nladdr, 0, sizeof(nladdr));
  memset(&msg, 0, sizeof(msg));

  nladdr.nl_family = AF_NETLINK;

  msg.msg_name = &nladdr;
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  iov.iov_base = nl_hdr;
  iov.iov_len = nl_hdr->nlmsg_len;
  ret = sendmsg(socketfd, &msg, 0);
  if (ret <= 0) {
    printf("Cannot send data to netlink socket (%d: %s)", errno, strerror(errno));
    return -1;
  }

  iov.iov_base = rcvbuf;
  iov.iov_len = sizeof(rcvbuf);
  ret = recvmsg(socketfd, &msg, 0);
  if (ret <= 0) {
    printf("Error while reading answer to netlink message (%d: %s)", errno, strerror(errno));
    return -1;
  }

  h = (struct nlmsghdr *)(void *)(rcvbuf);
  if (!NLMSG_OK(h, (unsigned int)ret)) {
    printf("Received netlink message was malformed (ret=%d, %u)", ret, h->nlmsg_len);
    return -1;
  }

  if (h->nlmsg_type != NLMSG_ERROR) {
    printf("Received unknown netlink response: %u bytes, type %u (not %u) with seqnr %u and flags %u from %u",
        h->nlmsg_len, h->nlmsg_type, NLMSG_ERROR, h->nlmsg_seq, h->nlmsg_flags, h->nlmsg_pid);
    return -1;
  }

  if (NLMSG_LENGTH(sizeof(struct nlmsgerr)) > h->nlmsg_len) {
    printf("Received invalid netlink message size %lu != %u",
        (unsigned long int)sizeof(struct nlmsgerr), h->nlmsg_len);
    return -1;
  }

  l_err = (struct nlmsgerr *)NLMSG_DATA(h);

  if (l_err->error) {
    printf("Received netlink error code %s (%d)", strerror(-l_err->error), l_err->error);
  }
  return -l_err->error;
}

/* For parsing the route info returned */
void parseRoutes(struct nlmsghdr *nlHdr, struct route_entry *rtInfo)
{
  struct rtmsg *rtMsg;
  struct rtattr *rtAttr;
  int rtLen;
  char *tempBuf = NULL;

  tempBuf = (char *)malloc(1024);
  rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

  /* get the rtattr field */
  rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
  rtLen = RTM_PAYLOAD(nlHdr);
  for (; RTA_OK(rtAttr,rtLen); rtAttr = RTA_NEXT(rtAttr,rtLen)) {
    switch(rtAttr->rta_type) {
            case RTA_OIF:
                    rtInfo->ifindex = *((int *)RTA_DATA(rtAttr));
                    break;
            case RTA_GATEWAY:
                    rtInfo->gateway = *((u_int *)RTA_DATA(rtAttr));
                    break;
            case RTA_PREFSRC:
                    rtInfo->src = *((u_int *)RTA_DATA(rtAttr));
                    break;
            case RTA_DST:
                    rtInfo->dst = *((u_int *)RTA_DATA(rtAttr));
                    break;
            case RTA_TABLE:
                    rtInfo->table = *((u_int *)RTA_DATA(rtAttr));
                    break;
            case RTA_PRIORITY:
                    rtInfo->metric = *((u_int *)RTA_DATA(rtAttr));
                    break;
            default:
              printf("type is %d not found.\n", rtAttr->rta_type);
                break;
    }
  }
  free(tempBuf);
}

static int
netlink_send_get_request(int socketfd, struct nlmsghdr *nl_hdr, route_entry* table, int* len)
{
  char rcvbuf[1024];
  struct iovec iov;
  struct sockaddr_nl nladdr;
  struct msghdr msg;
  struct nlmsghdr *h;
  int ret;

  memset(&nladdr, 0, sizeof(nladdr));
  memset(&msg, 0, sizeof(msg));

  nladdr.nl_family = AF_NETLINK;

  msg.msg_name = &nladdr;
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  iov.iov_base = nl_hdr;
  iov.iov_len = nl_hdr->nlmsg_len;
  ret = sendmsg(socketfd, &msg, 0);
  if (ret <= 0) {
    printf("Cannot send data to netlink socket (%d: %s)", errno, strerror(errno));
    return -1;
  }

  iov.iov_base = rcvbuf;
  iov.iov_len = sizeof(rcvbuf);

  bool end_flag = false;
  int entry_number = 0;
  route_entry* head = NULL;
  route_entry* tail = NULL;   

  while(!end_flag){
    memset(rcvbuf, 0, 1024);
    ret = recvmsg(socketfd, &msg, 0);
    if (ret <= 0) {
      printf("Error while reading answer to netlink message (%d: %s)", errno, strerror(errno));
      return -1;
    }

    h = (struct nlmsghdr *)(void *)(rcvbuf);

    if (!NLMSG_OK(h, (unsigned int)ret)) {
      printf("Received netlink message was malformed (ret=%d, %u)", ret, h->nlmsg_len);
      return -1;
    }

    
    for (;NLMSG_OK(h, ret); h = NLMSG_NEXT(h, ret)){
      switch (h->nlmsg_type){
        case NLMSG_DONE:
                end_flag = true;
                break;
        case RTM_NEWROUTE:
            struct route_entry *rtEntry = (struct route_entry*)malloc(sizeof(struct route_entry));
            memset(rtEntry, 0, sizeof(struct route_entry));
            parseRoutes(h, rtEntry);
            memcpy(table + entry_number * sizeof(struct route_entry), rtEntry, sizeof(struct route_entry));
            free(rtEntry);
            entry_number ++;
            break;
      }
    }
  }

  *len = entry_number;

  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////


int init_fo_socket(){
    if(fo_socket_fd != 0){
        //FIXME:need record in a log
        printf("fo_socket_fd exist.\n");
        return -1;
    }

    fo_socket_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(fo_socket_fd < 0){
        printf("create a fo_socket failed.\n");
        return -1;
    }

    return 0;
}

void close_fo_socket(){
    close(fo_socket_fd);
}

int FO_add_a_route(uint32_t dst, int netmask, uint32_t gateway, int ifindex, int metric, uint32_t src){
    struct rtreq req;
    memset(&req, 0, sizeof(req));

    req.r.rtm_family = family;
    req.r.rtm_flags = flags;
    req.r.rtm_table = rttable;
    req.r.rtm_type = RTN_UNICAST;
    req.r.rtm_dst_len = netmask;//TIP: this is dst's mask length,here is 32
    req.r.rtm_protocol = protocol;
    req.r.rtm_scope = scope;

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_flags |= NLM_F_CREATE | NLM_F_REPLACE;
    req.n.nlmsg_type = RTM_NEWROUTE;

    netlink_addreq(&req.n, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));

    if (metric >= 0) {
        netlink_addreq(&req.n, sizeof(req), RTA_PRIORITY, &metric, sizeof(metric));
    }

    in_addr src_net, dst_net, gwc_net;
    dst_net.s_addr = dst;
    gwc_net.s_addr = gateway;
    if(src != 0){
        src_net.s_addr = src;
        netlink_addreq(&req.n, sizeof(req), RTA_PREFSRC, &src_net, family_size);
    }
    /* add gateway*/
    netlink_addreq(&req.n, sizeof(req), RTA_GATEWAY, &gwc_net, family_size);
    /* add destination */
    netlink_addreq(&req.n, sizeof(req), RTA_DST, &dst_net, family_size);

    int err = netlink_send_update_request(fo_socket_fd, &req.n);
    if (err) {
        printf("failed\n");
        return -1;
    }
    
    return 0;
}

int FO_delete_a_route(uint32_t dst, uint32_t gateway, int ifindex, int metric, uint32_t src){
    if(dst == 0 && gateway ==0 && ifindex < 0){
        printf("delete no target.\n");
        return -1;
    }

    struct rtreq req;
    memset(&req, 0, sizeof(req));

    req.r.rtm_family = family;
    req.r.rtm_flags = flags;
    req.r.rtm_table = rttable;
    req.r.rtm_type = RTN_UNICAST;
    req.r.rtm_dst_len = sizeof(struct in_addr) * 8;//TIP: this is dst's mask length,here is 32

    req.r.rtm_scope = scope;

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_type = RTM_DELROUTE;

    if(ifindex >=0){
        netlink_addreq(&req.n, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));
    }

    if (metric >= 0) {
        netlink_addreq(&req.n, sizeof(req), RTA_PRIORITY, &metric, sizeof(metric));
    }

    in_addr src_net, dst_net, gwc_net;
    dst_net.s_addr = dst;
    gwc_net.s_addr = gateway;
    if(src){
        src_net.s_addr = src;
        netlink_addreq(&req.n, sizeof(req), RTA_PREFSRC, &src_net, family_size);
    }
    /* add gateway*/
    if(gateway){
        netlink_addreq(&req.n, sizeof(req), RTA_GATEWAY, &gwc_net, family_size);
    }
    
    /* add destination */
    if(dst){
        netlink_addreq(&req.n, sizeof(req), RTA_DST, &dst_net, family_size);
    }
    
    int err = netlink_send_update_request(fo_socket_fd, &req.n);
    if (err) {
        printf("failed\n");
        return -1;
    }
    
    return 0;
}

int FO_get_routes(route_entry* table, int* len){
    struct rtreq req;
    memset(&req, 0, sizeof(req));

    req.r.rtm_family = AF_INET;
    req.r.rtm_flags = 0;
    req.r.rtm_table = rttable;
    req.r.rtm_type = 0;
    req.r.rtm_dst_len = 0; // IPv4 sizeof() return bytes, rtm_dst_len need mask(bit)
    req.r.rtm_protocol = 0;
    req.r.rtm_scope = 0;

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.n.nlmsg_type = RTM_GETROUTE;

    //send massage
    int err = netlink_send_get_request(fo_socket_fd, &req.n, table, len);
    if (err) {
        printf("get route failed\n");
        return -1;
    }

    return 0;
}

u_int64_t getcurrtime()
{
  struct timeval tv;

  if (gettimeofday(&tv, NULL) < 0)
    /* Couldn't get time of day */
    return -1;

  return ((u_int64_t)tv.tv_sec) * 1000 + ((u_int64_t)tv.tv_usec) / 1000;
}

int FO_query_route_idle_time(addr_t dest, int recv_flag){
	FILE *fp;
	char sip[20];
	char title[80];
	int	 vFlag;
	u_int32_t ip ;
	// FIXME : I do not know how to scang u_int64_t 
	//u_int64_t last_use_time ;
	unsigned long last_use_time=0;
	int		idle_time = -1;
	struct in_addr inp;
	
	fp=fopen("/proc/asl/route_check","r");
	if(fp==NULL)
	{
		fprintf(stderr, "Probably the route_check module has not been insmod\n");
		perror("Opening the proc file");
		return -1 ;
	}
		
	/* ignore the first line */
	fgets(title, 120, fp); 
	
	while(fscanf(fp, "%s %lu %d\n", sip, &last_use_time, &vFlag) != EOF)
	{
#ifdef DEBUG
		fprintf(stderr, "query_route_idle_time() : %s %lu %d\n", sip, last_use_time, vFlag);
#endif
		if( inet_aton(sip, &inp) == 0)
		{
			perror("inet_aton: Invalid address");
			return -1;
		}
		
		ip=inp.s_addr ;
		
		if((ip==dest) && (recv_flag == vFlag))
		{
			idle_time = getcurrtime() - last_use_time;
		}
	}
	
	fclose(fp);
	return idle_time ;	
}





