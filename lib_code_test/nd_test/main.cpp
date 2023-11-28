#include <iostream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <string.h>

#include "nd.h"

using namespace std;

void close_function(int type){
	nd_turn_down();
	exit(0);
}

int main(){
    signal(SIGINT, close_function);


	int ret = nd_turn_up();

	while(1){

	}


    return 0;
}