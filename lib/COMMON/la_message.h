#ifndef LA_MESSAGE_H
#define LA_MESSAGE_H

#include <iostream>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

#include "la_common.h"

using namespace std;


class LAMessage{
    public:
    //命令类型
    int cmd;

    //以下为命令内容

    //目标地址
    uint32_t dst_address;
    uint32_t src_address;
    uint8_t  protocol;

    //src和result只在LA_ODS_REPLY中使用
    int result;

    LAMessage();
    ~LAMessage();

    int generate_stream(char* buffer, int size);
    int parse_stream(char* buffer);
};

#endif