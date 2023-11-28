#include "la_select.h"

int la_fdset_init(){
    fd_list.clear();

    //加入ods服务套接字
    if(ods_fd < 0){
        printf("ODS socket hasn't gotten\n");
    }else {
        fd_list.push_back(ods_fd);
    }
    
    //加入服务器监听套接字
    fd_list.push_back(command_server_fd);

    return 0;
}

int la_fdset_update(int type, int fd){
    switch (type)
    {
        case FD_ADD:
            fd_list.push_back(fd);
            break;
        
        case FD_DEL:
            for(vector<int>::iterator i=fd_list.begin(); i != fd_list.end(); i++){
                if(*i == fd){
                    fd_list.erase(i);
                    break;
                }
            }
            break;
        default:
            break;
    }

    return 0;
}

void la_fdset_register(){
    fd_num = 0;
    FD_ZERO(&rbits_fd);

    for(int i=0;i<fd_list.size();i++){
        FD_SET(fd_list[i], &rbits_fd);
        if(fd_list[i] >= fd_num){
            fd_num = fd_list[i] + 1;
        }
        printf("%d\n",fd_list[i]);
    }
}

void la_main_loop(){
    while(1){
        printf("enter loop\n");
        la_fdset_register();
        printf("enter loop2\n");

        int ret;
        if( (ret = select(fd_num, &rbits_fd, NULL, NULL, NULL)) < 0 && errno != EINTR){
			printf("[la_main_loop] select failed, errno is %d\n", errno);
            return;
		}else{
            printf("enter loop3\n");
			for(int i=0;i<fd_list.size();i++){
				if(FD_ISSET(fd_list[i], &rbits_fd)){
                    if(fd_list[i] == ods_fd){
                        //处理ods消息
                        printf("enter loop4\n");
                        la_ods_message_handle();
                    }else if(fd_list[i] == command_server_fd){
                        //处理accept消息，即新的客户端连接
                        printf("enter loop5\n");
                        la_command_server_accept();
                    }else{
                        //处理客户端消息
                        printf("enter loop6\n");
                        la_serv_handle_message(fd_list[i]);
                    }
				}
			}
		}
    }
}

static void la_ods_message_handle(){
    printf("get a kernel message\n");
    //处理客户端发送的请求
    route_request now_request;
    int ret = ODS_get_route_request(&now_request);

    in_addr dst_net,src_net;
    dst_net.s_addr = now_request.dst_ip;
    src_net.s_addr = now_request.src_ip;
    printf("dst ip is %s\t", inet_ntoa(dst_net));
    printf("src ip is %s\t", inet_ntoa(src_net));
    printf("protocol is %d\n", now_request.protocol);
    
    //FIXME：查看到来的路由请求应该转发给哪一个路由守护进程，即查询路由守护进程管理的网段
    //在当前版本我们先转发给第一个客户端列表中的路由守护进程
    for(int i=0;i<la_serv_list.size();i++){
        if(la_serv_list[i].type == LA_CLIENT_DAEMON){
            //生成消息并传输
            LAMessage now_message;
            now_message.cmd = LA_ODS_REQUEST;
            now_message.dst_address = dst_net.s_addr;
            now_message.src_address = src_net.s_addr;
            now_message.protocol = now_request.protocol;
            now_message.result = 0;

            char buffer[1024];
            bzero(buffer, 1024);
            int message_len = now_message.generate_stream(buffer, 1024);
            int send_ret = send(la_serv_list[i].sock_fd, buffer, message_len, 0);
            if(send_ret < 0){
                printf("send LA_ODS_REQUEST to client socket %d failed\n", la_serv_list[i].sock_fd);
            }
            break;
        }
    }

}