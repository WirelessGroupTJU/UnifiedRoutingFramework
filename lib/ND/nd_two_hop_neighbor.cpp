#include "nd_two_hop_neighbor.h"

TwoHopEntry::TwoHopEntry(uint32_t ip, uint32_t gateway, uint8_t status){
    ip_address = ip;
    gateway_address = gateway;
    status_value = status;
}

TwoHopEntry::~TwoHopEntry(){
    
}