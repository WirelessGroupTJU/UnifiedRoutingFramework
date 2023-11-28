#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <string.h>
#include "../../lib/ODS/ods_userspace_netlink.h"
#include "../../lib/FO/fo.h"
#include "../../lib/ND/nd.h"

using namespace std;

void close_function(int type){
	nd_turn_down();
	exit(0);
}

//持续sleep sec秒，该睡眠不会被信号中断
void sleep_no_wake(int sec){  
    do{          
        sec = sleep(sec);
    }while(sec > 0);             
}

int main(){
    signal(SIGINT, close_function);


	int ret = nd_turn_up("configure.ini");

	sleep_no_wake(10);
	nd_set_hello_interval("ens33", 4000);
	sleep_no_wake(20);

	nd_set_hello_interval("ens33", 0);
	sleep_no_wake(30);

	nd_send_hello_now("ens33");
	nd_send_hello_now("ens33");
	nd_send_hello_now("ens33");

	sleep_no_wake(30);
	nd_set_hello_interval("ens33", 3000);
	while(1){
		
	}


    return 0;
}