#include "la_message.h"

LAMessage::LAMessage(){

}

LAMessage::~LAMessage(){

}

//将对象转化为字节流
int LAMessage::generate_stream(char* buffer, int size){
    memset(buffer,0,size);
    int len = 0;
    memcpy(buffer+len, &cmd, sizeof(cmd));
    len += sizeof(cmd);

    memcpy(buffer+len,&dst_address, sizeof(dst_address));
    len += sizeof(dst_address);

    memcpy(buffer+len,&src_address, sizeof(src_address));
    len += sizeof(src_address);

    memcpy(buffer+len,&protocol, sizeof(protocol));
    len += sizeof(protocol);

    memcpy(buffer+len,&result, sizeof(result));
    len += sizeof(result);

    return len;
}

int LAMessage::parse_stream(char* buffer){
    int cmd_value;
    uint32_t dst_address_value, src_address_value;
    uint8_t protocol_value;
    int result_value;

    int len = 0;
    memcpy(&cmd_value, buffer+len, sizeof(cmd_value));
    len += sizeof(cmd_value);

    memcpy(&dst_address_value, buffer+len, sizeof(dst_address_value));
    len += sizeof(dst_address_value);

    memcpy(&src_address_value, buffer+len, sizeof(src_address_value));
    len += sizeof(src_address_value);

    memcpy(&protocol_value, buffer+len, sizeof(protocol_value));
    len += sizeof(protocol_value);

    memcpy(&result_value, buffer+len,sizeof(result_value));
    len += sizeof(result_value);

    cmd = cmd_value;
    dst_address = dst_address_value;
    src_address = src_address_value;
    protocol = protocol_value;
    result = result_value;
    return 0;
}