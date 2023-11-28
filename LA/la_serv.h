#ifndef LA_SERV_H
#define LA_SERV_H

#include <iostream>
#include <net/if.h>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "common.h"

using namespace std;

#define COMMAND_PORT 12001
#define FD_ADD 0
#define FD_DEL 1

//该结构体用于对路由守护进程的抽象描述
class LAServ{

public:
    //server端用于和客户端通信的描述符
    int sock_fd;

    //标识该客户端的类型（路由守护进程 or 配置终端）
    int type;

    //下面就是对一些网络接口或者路由守护进程的特点描述

    //守护进程管理的网络接口的描述
    int if_index;
    char if_name[IF_NAMESIZE];
    struct sockaddr if_addr;

    LAServ();
    ~LAServ();
};

extern vector<LAServ> la_serv_list;

//服务器监听套接字
extern int command_server_fd;

extern vector<int> fd_list;

extern int la_fdset_update(int type, int fd);

int la_command_server_socket_init();
void la_command_server_socket_close();
//对两种socket的处理
int la_command_server_accept();
int la_serv_handle_message(int fd);

int sockopt_reuseaddr (int sock);
int sockopt_reuseport (int sock);


#endif