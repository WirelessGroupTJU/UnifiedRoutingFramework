#ifndef ONE_HOP_NEIGHBOR_H
#define ONE_HOP_NEIGHBOR_H

#include <iostream>
#include <vector>
#include "nd_two_hop_neighbor.h"
#include "nd_type.h"

using namespace std;

class OneHopEntry
{
private:

public:
    uint32_t ip_address;
    
    //link quality
    int quality;

    //other host's local interface information
    vector<uint32_t> local_if_set;

    //this neighbor's one-hop neighbor
    vector<TwoHopEntry> two_hop_neighbor_set;

    //link sysmetric status
    uint8_t status;

    //entry live time
    uint32_t live_time;

    OneHopEntry(uint32_t ip_value, int quality_value
                , vector<uint32_t > &local_if_set_value, vector<TwoHopEntry > &two_hop_neighbor_set_value
                , int status_value, uint32_t live_time_value);
    ~OneHopEntry();
};



#endif