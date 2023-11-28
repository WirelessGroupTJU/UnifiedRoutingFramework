#ifndef ND_TWO_HOP_NEIGHBOR_H
#define ND_TWO_HOP_NEIGHBOR_H

#include <iostream>
#include "nd_type.h"

using namespace std;

class TwoHopEntry
{
private:

public:
    uint32_t ip_address;
    uint32_t gateway_address;
    uint8_t status_value;

    TwoHopEntry(uint32_t ip, uint32_t gateway, uint8_t stauts);
    ~TwoHopEntry();
};


#endif