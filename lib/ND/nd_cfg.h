#ifndef ND_COMMON_H
#define ND_COMMON_H

#include <iostream>
#include <net/if.h>
// #include <boost/property_tree/ptree.hpp>
// #include <boost/property_tree/ini_parser.hpp>

typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

using namespace std;
// using namespace boost;

//read from configure file
struct interface_pair{
    unsigned char name[IF_NAMESIZE];
    int listen_port;
    uint64_t hello_interval;
};



// static int read_configure_file(char* file_path, interface_pair interface_list[], int *listen_interface_number);




#endif