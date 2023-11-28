#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../lib/ODS/ods_userspace_netlink.h"
#include "../lib/FO/fo.h"
#include "../lib/ND/nd.h"
#include "../lib/COMMON/la_message.h"

using namespace std;

#define SERV_PORT 12001

void close_function(int type){
    printf("Ctrl+C,结束程序！\n");
	nd_turn_down();
	close_fo_socket();

	exit(0);
}

int main(int argc, char *argv[]){
    int sockfd, n;

    struct sockaddr_in servaddr;

    /* 创建套接字 */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket eror");

    /* 构建结构体ip、port */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        printf("inet_pton error");

    /* 建立连接 */
    int c = connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    if(c == -1) {
        printf("connnect error\n");
    }

    char* message = "hello engine\n";
    int ret = send(sockfd, message, strlen(message), 0);
    printf("%s\n", message);


    printf("client socket is %d\n", sockfd);
    int fo_ret = init_fo_socket();
    printf("fo socket is %d\n",fo_ret);
    //插入一条默认路由，用于ODS模块
    // char *dst = "1.2.3.4";
    // char *gateway = "1.2.3.5";
    // int ifindex = if_nametoindex("ens33");
    
    // int metric = 10;

    // uint32_t dst_net,gateway_net;
    // dst_net = inet_addr(dst);
    // gateway_net = inet_addr(gateway);

    // int re = FO_add_a_route(dst_net, 32, gateway_net, ifindex, metric, 0);

    signal(SIGINT, close_function);
    int nd_ret = nd_turn_up("./configure.ini");

    // printf("here\n");
    // debug();
    // printf("here\n");
    
    while(1){
        char recvbuf[1024];
        memset(recvbuf, 0, 1024);
        ret = recv(sockfd, recvbuf, 1024, 0);

        LAMessage now_message;
        now_message.parse_stream(recvbuf);

        printf("cmd is %d\n", now_message.cmd);

        struct in_addr addr;
        addr.s_addr = now_message.dst_address;
        printf("address is %s\n", inet_ntoa(addr));

        LAMessage feedback_message;
        feedback_message.cmd = LA_ODS_REPLY;
        feedback_message.dst_address = now_message.dst_address;
        feedback_message.result = IP_NOFOUND;
        
        char send_buf[1024];
        int len = 0;
        len = feedback_message.generate_stream(send_buf, 1024);
        int send_ret = send(sockfd, send_buf, len, 0);

    }

    close(sockfd);
    return 0;
}