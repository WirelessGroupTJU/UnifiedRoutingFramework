#include "nd.h"

//for test
// struct interface_pair interface_list[2] = {
//     {"ens33", 12001, 2000}
// };
// struct interface_pair *interface_list;
vector<interface_pair> interface_list;

vector<InterfaceEntry> interface_entry_list;

timerQueue timer_Q;

pthread_t thread_id;

/*parse the configure file*/
int parse_configure(char *cfg_path){
    //格式
    /*
        仿照ini配置文件
        空行
        ；代表注释
        name，port，hello_interval必须紧挨着
        等号前后有一个空格
    */

    // 读取ini文件
    FILE *cfg_file = fopen(cfg_path, "r");
    if(cfg_file == NULL){
        printf("read cfg file failed\n");
        return -1;
    }

    char buf[100];

    while(fgets(buf,100, cfg_file) != NULL){
        if(strlen(buf) == 1)
            continue;

        if(buf[0] == ';')
            continue;
        
        char ifname[16];
        bzero(ifname,0);
        int port;
        int hello_interval;

        fscanf(cfg_file, "name = %s\n",ifname);
        fscanf(cfg_file, "port = %d\n",&port);
        fscanf(cfg_file, "hello_interval = %d\n",&hello_interval);
        

        struct interface_pair now_pair;
        bzero(&now_pair, sizeof(struct interface_pair));
        memcpy(&(now_pair.name), ifname, strlen(ifname));
        now_pair.listen_port = port;
        now_pair.hello_interval = hello_interval;
        interface_list.push_back(now_pair);

        printf("name is %s\tport is %d\thello_interval is %lu\n", now_pair.name, port, hello_interval);
        
    }

    fclose(cfg_file);
    return 0;
}

/* initialize hello socket */
int init_socket (char *IF){
    int fd;
    int on = 1;
    int b_cast = 1;
    int	pri=0;
    int type = SOCK_DGRAM;

    /* Get socket for UDP or TCP */
    if ((fd = socket(AF_INET, type, 0)) == -1)
    /* Error creating socket */
    return -1;

    /* Setting socket to only listen to out device (and not loopback) */
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, IF,
            (size_t)((strlen(IF)+1)*sizeof(char))) < 0)
    /* Error in setting socket */
    return -1;

    /* Setting socket so that more than one process can use the address */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    /* Error in setting socket */
    return -1;

    /* Listen to broadcast as well? */
    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &b_cast, sizeof(b_cast)) < 0)
    /* Error in setting socket */
    return -1;

    /* set socket priority */
    if (setsockopt(fd, SOL_SOCKET, SO_PRIORITY , &pri, sizeof(pri)) < 0)
        return -1;

    /* set option to receive IP_TTL for received messages. */
    if (setsockopt(fd, SOL_IP, IP_RECVTTL, &on, sizeof(on)) < 0)
    return -1;

    /* set option to receive PKTINFO for received messages. */
    if (setsockopt(fd, SOL_IP, IP_PKTINFO, &on, sizeof(on)) < 0)
    return -1;

    return fd;
}

int bind_socket (int fd, int listen_port, struct sockaddr_in *addr){
	  addr->sin_family = AF_INET;
	  addr->sin_port = htons(listen_port);
	  bzero(&(addr->sin_zero), 8);
	  addr->sin_addr.s_addr=INADDR_ANY;

	  /* bind hello socket to my address */
	  if (bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr)) == -1)
	    return -1;

	  return 0;
}

int init_interface_entry_list(){
    interface_entry_list.clear();

    for(int i=0; i<interface_list.size(); i++){

        unsigned char* name = interface_list[i].name;
        uint64_t hello_interval = interface_list[i].hello_interval;
        int listen_port = interface_list[i].listen_port;
        in_addr interface_ipv4_address;

        int if_index = if_nametoindex((const char *)name);
        if(if_index == 0){
            printf("[init_interface_entry_list] interface not exist\n");
            return -1;
        }
        
        int socket_fd = init_socket((char*)name);
        if(socket_fd < 0){
            printf("[init_interface_entry_list] allocate hello socket failed\n");
            return -1;
        }

        struct sockaddr_in server_addr;
        int bind_ret = bind_socket(socket_fd, listen_port, &server_addr);
        if(bind_ret < 0){
            printf("[init_interface_entry_list] bind hello socket failed\n");
            return -1;
        }

        struct ifaddrs *ifap, *ifa;
        getifaddrs(&ifap);
        for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
            if(ifa->ifa_addr == NULL)
                continue;

            if(strcmp(ifa->ifa_name, (const char*)name) == 0 && ifa->ifa_addr->sa_family == AF_INET){
                interface_ipv4_address = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                break;
            }
            
        }
        freeifaddrs(ifap);
        #ifdef DEBUG
            printf("address is %s\n", inet_ntoa(interface_ipv4_address));
        #endif
        printf("nd socket is %d\n", socket_fd);
        InterfaceEntry now_entry(name, if_index, listen_port, interface_ipv4_address.s_addr, socket_fd, hello_interval);
        interface_entry_list.push_back(now_entry);

    }

    return 0;
}

void clean_interface_entry_list(){
    for(int i=0;i<interface_entry_list.size(); i++){
        close(interface_entry_list[i].fd);
    }
    interface_entry_list.clear();
}

/*
    name: main_loop()
    this function is making all fd into a while-true loop for sending or receiving hello message 
*/
void *main_loop(void *arg){
    int nfds=0, ret;
	fd_set	rbits, rfds;

    FD_ZERO(&rbits);

    for(int i=0; i < interface_entry_list.size(); i++){
		FD_SET(interface_entry_list[i].fd, &rbits);
		if(interface_entry_list[i].fd >= nfds)
			nfds = interface_entry_list[i].fd +1;
	}

    while(1){

		memcpy((char *)&rfds, (char *)&rbits, sizeof(rbits));

		if( (ret = select(nfds, &rfds, NULL, NULL, NULL)) < 0 && errno != EINTR){
			printf("[main_loop] select failed, errno is %d\n", errno);
            return NULL;
		}
		else{
			for(int i=0;i<interface_entry_list.size();i++){
				if(FD_ISSET(interface_entry_list[i].fd, &rfds)){
					interface_entry_list[i].handle_hello_message();
				}
			}
		}
	}
}

void send_hello_message(void *data){
    struct timerData* param = (struct timerData *)data;
    InterfaceEntry* entry = (InterfaceEntry *)param->contain;

    // printf("[%lu] <%s> send a hello message\n", timer_Q.getcurrtime(),entry->name);
    int len = entry->generate_and_send_hello_message();

    //when hello interval == 0, don't add a new timer
    if(entry->hello_interval == 0){
        printf("<%s> enter on-demand send mode\n", entry->name);
    }else{
        struct	timerData	tData;
        tData.type = TIMER_TYPE_HELLO; 
        tData.data = 0;
        tData.contain = entry;

        timer_Q.set_timer(entry->hello_interval, (timer_hfunc_t)send_hello_message, &tData);
    }
    
}

void start_discovery(){
    for(int i=0;i<interface_entry_list.size();i++){
        struct	timerData	tData;
        tData.type = TIMER_TYPE_HELLO; 
        tData.data = 0;
        tData.contain = (void *)(&interface_entry_list[i]);

        send_hello_message(&tData);
    }
}

void delete_one_hop_entry(void *data){
    struct timerData* param = (struct timerData *)data;
    InterfaceEntry* entry = (InterfaceEntry *)param->contain;
    
    printf("<%s> delete a old one-hop entry\n", entry->name);
    debug();

    uint32_t neighbor_address = param->data;

    for(vector<OneHopEntry >::iterator it = entry->one_hop_neighbor_set.begin(); it != entry->one_hop_neighbor_set.end(); it ++){
        if((*it).ip_address == neighbor_address){
            entry->one_hop_neighbor_set.erase(it);
            break;
        }
    }

    debug();
}

//注册超时信号量与对应处理函数
void register_signal(){
    //定时器到时
    signal(SIGALRM, timeout_handle);
}

void timeout_handle(int type){
    timer_Q.scheduleTimer();
}

void debug(){
    printf("now time is %lu\n, this process has %d interface\n", timer_Q.getcurrtime(), interface_entry_list.size());
    for(int i=0;i<interface_entry_list.size();i++){
        printf("------This Interface is %s-----------\n", interface_entry_list[i].name);
        printf("\tneighbor number is %ld\n", interface_entry_list[i].one_hop_neighbor_set.size());
        for(int j=0;j<interface_entry_list[i].one_hop_neighbor_set.size();j++){
            in_addr src;
            src.s_addr = interface_entry_list[i].one_hop_neighbor_set[j].ip_address;
            printf("\t\taddress is %s\tstatus is %s\n", inet_ntoa(src), interface_entry_list[i].one_hop_neighbor_set[j].status == HEARD?"HEARD":"SYMMTRIC");
        }
    }
}

int nd_turn_up(char* cfg_path){
    int cfg_ret = parse_configure(cfg_path);
    if(cfg_ret < 0){
        printf("[nd_turn_up] read configure file failed\n");
        return -1;
    }

    int ret = init_interface_entry_list();
    if(ret < 0){
        printf("[nd_turn_up] init interface list failed\n");
        return -1;
    }

    register_signal();

    int thread_ret = pthread_create(&thread_id, NULL, main_loop, NULL);
    if(thread_ret != 0){
        printf("[nd_turn_up] main loop thread create failed\n");
        return -1;
    }

    //分离子线程
    pthread_detach(thread_id);
    
    start_discovery();

    return 0;
}

void nd_turn_down(){
    //kill main-loop thread
    pthread_cancel(thread_id);

    //release socket fd
    clean_interface_entry_list();

    //clean timer queue
    timer_Q.clean_timer_list();
}

int nd_set_hello_interval(char* name, uint64_t new_hello_interval){
    int index = -1;
    for(int i=0;i<interface_entry_list.size();i++){
        if(!strcmp((const char*)interface_entry_list[i].name, name)){
            index = i;
            break;
        }
    }

    if(index < 0){
        printf("[nd_set_hello_interval] not find this interface\n");
        return -1;
    }

    if(interface_entry_list[index].hello_interval != 0){
        interface_entry_list[index].hello_interval = new_hello_interval;
    }else {
        printf("[nd_set_hello_interval] interface <%s> enter Cycle transmission mode, new interval is %lu\n", interface_entry_list[index].name, new_hello_interval);

        interface_entry_list[index].hello_interval = new_hello_interval;
        struct	timerData	tData;
        tData.type = TIMER_TYPE_HELLO; 
        tData.data = 0;
        tData.contain = (void *)(&interface_entry_list[index]);

        send_hello_message(&tData);
    }
    return 0;
}

int nd_send_hello_now(char* name){
    int index = -1;
    for(int i=0;i<interface_entry_list.size();i++){
        if(!strcmp((const char*)interface_entry_list[i].name, name)){
            index = i;
            break;
        }
    }

    if(index < 0){
        printf("[nd_set_hello_interval] not find this interface\n");
        return -1;
    }

    int ret = interface_entry_list[index].generate_and_send_hello_message();
    printf("[nd_send_hello_now] send a hello message now\n");
    return ret;
}

void nd_get_one_hop_neighbor_information(char* name, vector<pair<uint32_t, uint8_t> >& neighbor_list){
    int index = -1;
    for(int i=0;i<interface_entry_list.size();i++){
        if(!strcmp((const char*)interface_entry_list[i].name, name)){
            index = i;
            break;
        }
    }

    if(index < 0){
        printf("[nd_set_hello_interval] not find this interface\n");
        return;
    }

    neighbor_list.clear();
    for(int i=0;i<interface_entry_list[index].one_hop_neighbor_set.size();i++){
        pair<uint32_t, uint8_t> now_pair;
        now_pair.first = interface_entry_list[index].one_hop_neighbor_set[i].ip_address;
        now_pair.second= interface_entry_list[index].one_hop_neighbor_set[i].status;
        neighbor_list.push_back(now_pair);
    }

}