#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <string.h>
#include "../lib/ODS/ods_userspace_netlink.h"
#include "../lib/FO/fo.h"
#include "../lib/ND/nd.h"

#include "la_select.h"
#include "la_serv.h"

void close_function(int type){
    printf("Ctrl+C,结束程序！\n");
	// nd_turn_down();
	close_fo_socket();
    ods_turn_down();

	exit(0);
}

//客户端列表
vector<LAServ> la_serv_list;

//服务器监听套接字
int command_server_fd;

//ods模块的套接字描述符
int ods_fd = -1;

//监听的fd集合
vector<int> fd_list;
fd_set rbits_fd;
//监听的fd总数
int fd_num;

//持续sleep sec秒，该睡眠不会被信号中断
void sleep_no_wake(int sec){  
    do{          
        sec = sleep(sec);
    }while(sec > 0);             
}

int main(){
    

    signal(SIGINT, close_function);
	// int nd_ret = nd_turn_up("configure.ini");
    int fo_ret = init_fo_socket();
    printf("fo socket is %d\n",fo_ret);
    ods_fd = ods_turn_up();
    printf("ods socket is %d\n", ods_fd);

    //插入一条默认路由，用于ODS模块
    char *dst = "0.0.0.0";
    char *gateway = "1.2.3.4";
    int ifindex = if_nametoindex("mytun");
    
    int metric = 10;

    uint32_t dst_net,gateway_net;
    dst_net = inet_addr(dst);
    gateway_net = inet_addr(gateway);

    int re = FO_add_a_route(dst_net, 0, gateway_net, ifindex, metric, 0);

    // //主线程屏蔽SIGALRM信号
    // sigset_t set;
    // sigemptyset(&set);
    // sigaddset(&set,SIGALRM);
    // pthread_sigmask(SIG_SETMASK,&set,NULL);


	int ret = la_command_server_socket_init();

    ret = la_fdset_init();

    // printf("here\n");
    // debug();
    
    // daemon(0,0);

    la_main_loop();
	


    return 0;
}