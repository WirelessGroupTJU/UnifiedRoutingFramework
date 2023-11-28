#ifndef ND_INTERFACE_H
#define ND_INTERFACE_H

#include <vector>
#include <net/if.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "nd_one_hop_neighbor.h"
#include "nd_timer_queue.h"

class InterfaceEntry
{
private:
    
public:
    unsigned char name[IF_NAMESIZE];
    int if_index;
    int listen_port;
    uint32_t interface_address;

    //listen socket fd
    int fd;

    //discovery interval
    uint64_t hello_interval;

    //store one-hop neighbor information
    vector<OneHopEntry> one_hop_neighbor_set;

    InterfaceEntry(unsigned char *name_value, int if_index_value, int listen_port_value, uint32_t interface_address_value
                , int fd_value, uint64_t hello_interval_value);

    ~InterfaceEntry();

    //when receive a hello message, need call this function, this function can be self-defined to suit for various policy
    void handle_hello_message();
    void update_info(uint32_t src_address
        , vector<uint32_t> &src_local_if_set
        , vector<uint32_t> &src_neighbor_set
        , vector<uint8_t > &src_status_set
        , uint32_t src_live_time);
    int read_from_sock(void *message_buffer, void *control_data);

    //based on the status of this interface's information at this time, we generate hello message and send it
    int generate_and_send_hello_message();
};

#endif