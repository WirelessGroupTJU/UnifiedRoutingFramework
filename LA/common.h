#ifndef LA_COMMON_H
#define LA_COMMON_H

#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <string.h>
#include "../lib/ODS/ods_userspace_netlink.h"
#include "../lib/FO/fo.h"
#include "../lib/ND/nd.h"
#include "../lib/COMMON/la_message.h"

using namespace std;

#define LA_CLIENT_DAEMON 0
#define LA_CLIENT_CLI 1


/*
    当前在本地模式中，路由守护进程和代理进程之间的传输信令类型有：
    1. LA_ODS_REQUEST 内核发送到用户空间中的路由请求，Local Agent需要将它转发到对应的路由守护进程中
    2. LA_ODS_REPLY 路由守护进程向Local Agent传递的路由处理结果
*/
#define LA_ODS_REQUEST 0
#define LA_ODS_REPLY 1

#endif