#include "nd_one_hop_neighbor.h"

OneHopEntry::OneHopEntry(uint32_t ip_value, int quality_value
                , vector<uint32_t > &local_if_set_value, vector<TwoHopEntry > &two_hop_neighbor_set_value
                , int status_value, uint32_t live_time_value){
    ip_address = ip_value;
    quality = quality_value;

    local_if_set.clear();
    local_if_set.assign(local_if_set_value.begin(), local_if_set_value.end());

    two_hop_neighbor_set.clear();
    two_hop_neighbor_set.assign(two_hop_neighbor_set_value.begin(), two_hop_neighbor_set_value.end());

    status = status_value;
    live_time = live_time_value;

}

OneHopEntry::~OneHopEntry(){
    local_if_set.clear();
    two_hop_neighbor_set.clear();
}