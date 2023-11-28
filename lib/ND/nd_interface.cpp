#include "nd_interface.h"

extern timerQueue timer_Q;
extern vector<InterfaceEntry > interface_entry_list;
extern void delete_one_hop_entry(void *data);
extern void debug();

InterfaceEntry::InterfaceEntry(unsigned char *name_value, int if_index_value, int listen_port_value, uint32_t interface_address_value
                , int fd_value, uint64_t hello_interval_value){
    
    strcpy((char*)name, (char*)name_value);
    if_index = if_index_value;
    listen_port = listen_port_value;
    interface_address = interface_address_value;

    fd = fd_value;
    hello_interval = hello_interval_value;

    one_hop_neighbor_set.clear();
}

InterfaceEntry::~InterfaceEntry(){
    one_hop_neighbor_set.clear();
}

int InterfaceEntry::generate_and_send_hello_message(){
    int len = 0;
    void *message_buffer = malloc(MESSAGE_BUFFER_SIZE);
    if(message_buffer == NULL){
        printf("[generate_and_send_hello_message] malloc message buffer failed\n");
        return -1;
    }
    memset(message_buffer, 0, MESSAGE_BUFFER_SIZE);

    //bytes
    unsigned char addr_length = IPV4_ADDR_LEN;
    memcpy(message_buffer + len, &addr_length, sizeof(addr_length));
    len += sizeof(addr_length);

    unsigned char local_if_number = interface_entry_list.size();
    memcpy(message_buffer + len, &local_if_number, sizeof(local_if_number));
    len += sizeof(local_if_number);
    
    unsigned char neighbor_number = one_hop_neighbor_set.size();
    memcpy(message_buffer + len, &neighbor_number, sizeof(neighbor_number));
    len += sizeof(neighbor_number);

    //FIX
    unsigned char check = 1;
    memcpy(message_buffer + len, &check, sizeof(check));
    len += sizeof(check);

    //add local_if_set
    for(int i=0;i<interface_entry_list.size();i++){
        uint32_t address = interface_entry_list[i].interface_address;
        memcpy(message_buffer + len, &address, sizeof(address));
        len += sizeof(address);
    }

    //add this entry's neighbor
    for(int i=0;i<one_hop_neighbor_set.size();i++){
        uint32_t neighbor_address = one_hop_neighbor_set[i].ip_address;
        memcpy(message_buffer + len, &neighbor_address, sizeof(neighbor_address));
        len += sizeof(neighbor_address); 
    }

    //add neighbor's status
    for(int i=0;i<one_hop_neighbor_set.size();i++){
        uint8_t status = one_hop_neighbor_set[i].status;
        memcpy(message_buffer + len, &status, sizeof(status));
        len += sizeof(status);
    }

    //add live time
    uint32_t live_time = LIVE_TIME;
    memcpy(message_buffer + len, &live_time, sizeof(live_time));
    len += sizeof(live_time);

    struct sockaddr_in dst;
    int ttl = 1;

    bzero((void *)&dst, sizeof(dst));
  	dst.sin_family = AF_INET;
  	dst.sin_port = htons(listen_port);     
    dst.sin_addr.s_addr = BROADCAST_ADDR;
    // dst.sin_addr.s_addr = inet_addr("192.168.16.255");

    if (setsockopt(fd, SOL_IP, IP_TTL, &(ttl), sizeof(ttl)) < 0){
        perror("Error setting TTL");
    }

    int ret;
    if ((ret = sendto(fd, message_buffer, len, 0, 
	     (struct sockaddr *)&dst, sizeof(struct sockaddr))) < 0) {
			perror("Error in sendto");
  	}

    free(message_buffer);
    return len;
}

void InterfaceEntry::handle_hello_message(){
    struct controlData control_data;

    char *message_buffer = (char*)malloc(MESSAGE_BUFFER_SIZE);
    if(message_buffer == NULL){
        printf("[handle_hello_message] malloc message buffer failed\n");
        return;
    }

    int len = read_from_sock(message_buffer, &control_data);
    if(len <= 0){
        printf("[handle_hello_message] read from socket failed\n");
        return;
    }

    if(control_data.src == interface_address){
        return;
    }

    in_addr src_addr;
    src_addr.s_addr = control_data.src;
    printf("<%s> receive a neighbor's hello message, src is %s, ttl is %d\n", name, inet_ntoa(src_addr), control_data.ttl);

    /*
        hello message contain
    */
    int now_len = 0;

    //get address length
    uint8_t addr_len;
    memcpy(&addr_len, message_buffer + now_len, sizeof(addr_len));
    now_len += sizeof(addr_len);
    // printf("addr_len is %d\n", addr_len);

    //get local interface number
    uint8_t local_if_number;
    memcpy(&local_if_number, message_buffer + now_len, sizeof(local_if_number));
    now_len += sizeof(local_if_number);
    // printf("local is number is %d\n", local_if_number);

    //get neighbor number
    uint8_t neighbor_number;
    memcpy(&neighbor_number, message_buffer + now_len, sizeof(neighbor_number));
    now_len += sizeof(neighbor_number);
    // printf("neighbor number is %d\n", neighbor_number);

    //get check number
    uint8_t check;
    memcpy(&check, message_buffer + now_len, sizeof(check));
    now_len += sizeof(check);

    //get neighbor's local if set
    vector<uint32_t >neighbor_local_if_set;
    neighbor_local_if_set.clear();
    for(int i=0;i<local_if_number;i++){
        //attension this type
        uint32_t address;
        memcpy(&address, message_buffer + now_len, addr_len);
        now_len += addr_len;

        neighbor_local_if_set.push_back(address);
    } 

    //get neighbor's neighbor set
    vector<uint32_t >neighbor_set;
    neighbor_set.clear();
    for(int i=0;i<neighbor_number;i++){
        uint32_t address;
        memcpy(&address, message_buffer + now_len, addr_len);
        now_len += addr_len;

        neighbor_set.push_back(address);
    }

    //get neighbor's status
    vector<uint8_t >status_set;
    status_set.clear();
    for(int i=0;i<neighbor_number;i++){
        uint8_t status;
        memcpy(&status, message_buffer + now_len, sizeof(status));
        now_len += sizeof(status);

        status_set.push_back(status);
    }

    //get live time
    uint32_t live_time;
    memcpy(&live_time, message_buffer + now_len, sizeof(live_time));
    now_len += sizeof(live_time);

    // printf("hello message len is %d, read len is %d\n", len, now_len);
    
    //use hello message's contain to update this interface's infobase
    update_info(control_data.src, neighbor_local_if_set, neighbor_set, status_set, live_time);
    
}

int InterfaceEntry::read_from_sock(void *message_buffer, void* control_data){
	int	len;
	struct sockaddr_in src_addr;
	struct msghdr msg;
    struct controlData *now_data = (struct controlData*)control_data;
    now_data->src = 0;
    now_data->ttl = 0;

	union control_union{
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int)) +
			     CMSG_SPACE(sizeof(struct in_pktinfo))];
	};

	union control_union control_un;

    struct cmsghdr *cmsg;
	char	*cmsgPtr;

	struct in_pktinfo pktinfo;
	socklen_t	addr_len;
	int	recvTTL=0;

	memset(&src_addr,0,sizeof(struct sockaddr_in));

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = NULL;
    msg.msg_iovlen = 0;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    /* Get the information control message first */
    if ((len = recvmsg(fd, &msg, MSG_PEEK)) < 0) {
        cout << "Error in recvmsg " << endl;
        return -1;
    }
    addr_len = sizeof(struct sockaddr_in);

    /* Read the data payload (i.e. hello msg) */
    len = recvfrom(fd, message_buffer, MESSAGE_BUFFER_SIZE, 0,
            (struct sockaddr *) &src_addr, &addr_len);

    if (len < 0) {
        cout << "aodv socket: error in recvfrom " << endl;
        return -1;
    }

    u_int32_t	src = (src_addr.sin_addr.s_addr);

    //explain control information
    cmsg = CMSG_FIRSTHDR(&msg);
    if(msg.msg_controllen == 56) {
        for (int i = 0; i < 2; i++) {
            if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_TTL) {
                memcpy(&recvTTL, CMSG_DATA(cmsg), sizeof(int));
                cmsgPtr = (char*)cmsg;
                cmsgPtr =  cmsgPtr + CMSG_SPACE(sizeof(int));
                cmsg = (struct cmsghdr *)cmsgPtr;
            } else if (cmsg->cmsg_level == SOL_IP
                && cmsg->cmsg_type == IP_PKTINFO) {
                memcpy(&pktinfo, CMSG_DATA(cmsg), sizeof(struct in_pktinfo));
                cmsgPtr = (char*)cmsg;
                cmsgPtr =  cmsgPtr + CMSG_SPACE(sizeof(struct in_pktinfo));
                cmsg = (struct cmsghdr *)cmsgPtr;
            }
        }
    }else
    	cout <<" read From socket: header not found " << endl;

    
    now_data->src = src;
    now_data->ttl = recvTTL;

#ifdef DEBUG
		cout << "readFromSocket: ttl received is " << recvTTL << endl;
#endif
	return len;
}

void InterfaceEntry::update_info(uint32_t src_address
    , vector<uint32_t> &src_local_if_set
    , vector<uint32_t> &src_neighbor_set
    , vector<uint8_t > &src_status_set
    , uint32_t src_live_time){
    
    uint8_t neighbor_status_in_host = NO_ENTRY;
    uint8_t host_status_in_message = NO_ENTRY;
    int index = -1;
    debug();

    //whether this neighbor already have a record in one_hop_neighbor_set  
    for(int i=0;i<one_hop_neighbor_set.size();i++){
        if(src_address == one_hop_neighbor_set[i].ip_address){
            neighbor_status_in_host = one_hop_neighbor_set[i].status;
            index = i;
            break;
        }
    }

    //whether this host already have been heard by this neighbor
    for(int i=0;i<src_neighbor_set.size();i++){
        if(interface_address == src_neighbor_set[i]){
            host_status_in_message = src_status_set[i];
            break;
        }
    }

    if(neighbor_status_in_host == NO_ENTRY){
        //generate two-hop set
        vector<TwoHopEntry > temp_two_hop_set;
        temp_two_hop_set.clear();
        for(int i=0;i<src_neighbor_set.size();i++){
            if(src_neighbor_set[i] != interface_address){
                TwoHopEntry two_entry(src_neighbor_set[i], src_address, src_status_set[i]);
                temp_two_hop_set.push_back(two_entry);
            }
        }

        if(host_status_in_message == NO_ENTRY){
            //generate a one-hop entry
            OneHopEntry one_entry(src_address, 0, src_local_if_set, temp_two_hop_set, HEARD, src_live_time);
            one_hop_neighbor_set.push_back(one_entry);
        }else{
            //generate a one-hop entry
            OneHopEntry one_entry(src_address, 0, src_local_if_set, temp_two_hop_set, SYMMETRIC, src_live_time);
            one_hop_neighbor_set.push_back(one_entry);
        }

        //set timer for deleting a timeout entry
        struct	timerData	tData;
        tData.type = TIMER_TYPE_ENTRY; 
        tData.data = src_address;
        tData.contain = (void *)(this);
        uint64_t src_live_time_64 = (uint64_t) src_live_time;
        timer_Q.set_timer(src_live_time_64 , (timer_hfunc_t)delete_one_hop_entry, &tData);
        
        debug();
        return;
    }

    if(neighbor_status_in_host == HEARD || neighbor_status_in_host == SYMMETRIC){
        //generate two-hop set
        vector<TwoHopEntry > temp_two_hop_set;
        temp_two_hop_set.clear();
        for(int i=0;i<src_neighbor_set.size();i++){
            if(src_neighbor_set[i] != interface_address){
                TwoHopEntry two_entry(src_neighbor_set[i], src_address, src_status_set[i]);
                temp_two_hop_set.push_back(two_entry);
            }
        }

        one_hop_neighbor_set[index].two_hop_neighbor_set.clear();
        one_hop_neighbor_set[index].two_hop_neighbor_set.assign(temp_two_hop_set.begin(), temp_two_hop_set.end());

        one_hop_neighbor_set[index].local_if_set.clear();
        one_hop_neighbor_set[index].local_if_set.assign(src_local_if_set.begin(), src_local_if_set.end());

        one_hop_neighbor_set[index].live_time = src_live_time;
        if(host_status_in_message == NO_ENTRY){
            one_hop_neighbor_set[index].status = HEARD;
        }else {
            one_hop_neighbor_set[index].status = SYMMETRIC;
        }

        //update timer
        //step1: delete old timer
        timer_Q.remove_neighbor_entry_timer(src_address, name);
        //add a new timer
        struct	timerData	tData;
        tData.type = TIMER_TYPE_ENTRY; 
        tData.data = src_address;
        tData.contain = (void *)(this);
        uint64_t src_live_time_64 = (uint64_t) src_live_time;
        timer_Q.set_timer(src_live_time_64 , (timer_hfunc_t)delete_one_hop_entry, &tData);

        debug();
        return;
    }
}