#ifndef LA_SELECT_H
#define LA_SELECT_H

#include <iostream>
#include <unistd.h>
#include "la_serv.h"
#include "common.h"

using namespace std;

extern int command_server_fd;
extern vector<LAServ> la_serv_list;

//ods模块的套接字描述符
extern int ods_fd;

//监听的fd集合
extern vector<int> fd_list;
extern fd_set rbits_fd;
//监听的fd总数
extern int fd_num;

//初始化描述符监听集合
int la_fdset_init();

//更新描述符监听集合（主要用于client连接的建立和断开）
int la_fdset_update(int type, int fd);

//注册
void la_fdset_register();

//主要调度循环
void la_main_loop();

//将内核传达的路由请求转发给守护进程
static void la_ods_message_handle();



#endif