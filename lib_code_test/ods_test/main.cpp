#include <iostream>
#include <signal.h>

#include "../../lib/ODS/ods_userspace_netlink.h"
#include "../../lib/FO/fo.h"

using namespace std;

void exit_function(int){
    ods_turn_down();
}

int main(){
    signal(SIGINT, exit_function);
    printf("hello ods test.\n");

    int ret = init_fo_socket();
    int nlfd = ods_turn_up();
    printf("nlfd is %d\n", nlfd);

    char *dst = "0.0.0.0";
    char *gateway = "1.2.3.4";
    int ifindex = if_nametoindex("mytun");
    
    int metric = 10;

    uint32_t dst_net,gateway_net;
    dst_net = inet_addr(dst);
    gateway_net = inet_addr(gateway);

    int re = FO_add_a_route(dst_net, 0, gateway_net, ifindex, metric, 0);


    // sleep(20);
    // //追加一条路由表项
    // int index = if_nametoindex("ens33");
    // re = FO_add_a_route(inet_addr("13.24.35.46"), 32, inet_addr("192.168.16.128"), index, metric, 0);
    // route_reply now_reply;
    // now_reply.now_ip = inet_addr("13.24.35.46");
    // now_reply.result = 1;
    // int result = ODS_feedback_route_reply(&now_reply);

    while(1){
        route_request now_request;
        int ret = ODS_get_route_request(&now_request);

        in_addr dst_net,src_net;
        dst_net.s_addr = now_request.dst_ip;
        src_net.s_addr = now_request.src_ip;
        printf("dst ip is %s\t", inet_ntoa(dst_net));
        printf("src ip is %s\t", inet_ntoa(src_net));
        printf("protocol is %d\n", now_request.protocol);

        if(now_request.input_ifindex != -1){
            char if_name[IF_NAMESIZE] = {'\0'};
            char *ret = if_indextoname(now_request.input_ifindex, if_name);
            printf("indev is %s\n", if_name);
        }else {
            printf("indev is Local\n");
        }


        sleep(15);
        //追加一条路由表项
        int index = if_nametoindex("ens33");
        int re = FO_add_a_route(now_request.dst_ip, 32, inet_addr("192.168.16.128"), index, metric, 0);
        // int re = FO_add_a_route(now_request.dst_ip, 32, inet_addr("6.7.8.9"), index, metric, 0);

        sleep(30);
        route_reply now_reply;
        now_reply.now_ip = now_request.dst_ip;
        now_reply.result = 1;
        int result = ODS_feedback_route_reply(&now_reply);
    }
    // ods_turn_down();

    return 0;
}



// int main(){
//       struct ifaddrs *ifap, *ifa;
//   struct sockaddr_in *sa;
//   char *addr;

//   getifaddrs(&ifap);

//   for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
//       if (ifa->ifa_addr->sa_family == AF_INET) { //IPv4地址
//           sa = (struct sockaddr_in *) ifa->ifa_addr;
//           addr = inet_ntoa(sa->sin_addr);
//           printf("%s - %s\n", ifa->ifa_name, addr);
//       }
//   } 

//   freeifaddrs(ifap);


//     return 0;
// }