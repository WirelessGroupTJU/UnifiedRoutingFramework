#ifndef ND_COMMON_H
#define ND_COMMON_H

#include <iostream>
#include <net/if.h>

using namespace std;

//read from configure file
struct interface_pair{
    unsigned char name[IF_NAMESIZE];
    int listen_port;
    uint64_t hello_interval;
};



// static int read_configure_file(char* file_path, interface_pair interface_list[], int *listen_interface_number);




#endif