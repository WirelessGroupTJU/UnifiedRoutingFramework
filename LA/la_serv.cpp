#include "la_serv.h"

LAServ::LAServ(){
    
}

LAServ::~LAServ(){

}

int la_command_server_socket_init(){
    int ret;
    int accept_sock;
    struct sockaddr_in addr;

    accept_sock = socket (AF_INET, SOCK_STREAM, 0);

    if (accept_sock < 0){
        printf("Can't create command server socket\n"); 
        return -1;
    }

    memset (&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(COMMAND_PORT);
    addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    // addr.sin_addr.s_addr = inet_addr("192.168.16.131");

    sockopt_reuseaddr (accept_sock);
    sockopt_reuseport (accept_sock);

    // 设置监听套接字为非阻塞模式
    int flags = fcntl(accept_sock, F_GETFL, 0);
    fcntl(accept_sock, F_SETFL, flags | O_NONBLOCK);
        
    ret  = bind (accept_sock, (struct sockaddr *)&addr, sizeof (struct sockaddr_in));
    if (ret < 0){
        printf("Can't bind to stream socket: %s", strerror (errno));
        printf("zebra can't provice full functionality due to above error");
        close (accept_sock);      /* Avoid sd leak. */
        return -1;
    }

    ret = listen (accept_sock, 5);
    if (ret < 0){
        printf("Can't listen to stream socket: %s", strerror (errno));
        printf("zebra can't provice full functionality due to above error");
        close (accept_sock);	/* Avoid sd leak. */
        return -1;
    }

    command_server_fd = accept_sock;
    printf("listen socket is %d\n", command_server_fd);
    return 0;
}

void la_command_server_socket_close(){
    close(command_server_fd);
}

int sockopt_reuseaddr (int sock){
    int ret;
    int on = 1;

    ret = setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, 
                (void *) &on, sizeof (on));
    if (ret < 0){
        printf("can't set sockopt SO_REUSEADDR to socket %d", sock);
        return -1;
    }
    return 0;
}


int sockopt_reuseport (int sock){
    int ret;
    int on = 1;

    ret = setsockopt (sock, SOL_SOCKET, SO_REUSEPORT, 
                (void *) &on, sizeof (on));
    if (ret < 0){
        printf("can't set sockopt SO_REUSEPORT to socket %d", sock);
        return -1;
    }
    return 0;
}

int la_command_server_accept(){
    //接收到新的连接请求
    
    struct sockaddr_in client;
    socklen_t clen = sizeof(client);
    int client_sock = accept(command_server_fd, (struct sockaddr*)&client, &clen);
    if(client_sock == -1){
        if(errno == EAGAIN || errno == EWOULDBLOCK){
            return 0;
        }else {
            return -1;
        }
    }
    //构建新的客户端描述
    LAServ now_client;
    now_client.sock_fd = client_sock;
    la_serv_list.push_back(now_client);

    //更新描述符集合
    la_fdset_update(FD_ADD, client_sock);

    printf("get a accpet socket %d !\n", client_sock);
    return 0;
}


int la_serv_handle_message(int fd){
    //处理客户端发送的请求
    printf("handle message\n");
    char recvbuf[1024];
    bzero(recvbuf, 1024);
    
    int recvlen = recv(fd, recvbuf, sizeof(recvbuf) - 1, 0);
    if(recvlen > 0){
        printf("data!\n");
        //解析数据
        LAMessage now_message;
        now_message.parse_stream(recvbuf);

        switch (now_message.cmd){
            case LA_ODS_REQUEST:
                /* code */
                break;
            case LA_ODS_REPLY:
                //路由守护进程传递来了处理结果，下面要向内核传递处理结果
                {
                printf("get a ODS reply message!\n");
                uint32_t target_address = now_message.dst_address;
                int result = now_message.result;
                printf("cmd is %d\n", LA_ODS_REPLY);

                in_addr addr;
                addr.s_addr = now_message.dst_address;
                printf("target address is %s\n", inet_ntoa(addr));
                printf("result is %s\n",result == 1?"IP_FOUND":"IP_NOTFOUND");

                route_reply now_reply;
                now_reply.now_ip = now_message.dst_address;
                now_reply.result = result;
                int ret = ODS_feedback_route_reply(&now_reply);
                break;
                }
            default:
                printf("OTHER CLIENT MESSAGE: %s\n", recvbuf);
                break;
        }

    }else{
        //连接关闭
        for(vector<LAServ>::iterator i=la_serv_list.begin(); i!=la_serv_list.end(); i++){
            if((*i).sock_fd == fd){
                close(fd);
                la_serv_list.erase(i);
                break;
            }
        }
        la_fdset_update(FD_DEL, fd);
        printf("close client socket %d\n", fd);
    }
    
    return 0;
}

