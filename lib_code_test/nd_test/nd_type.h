#ifndef ND_TYPE_H
#define ND_TYPE_H

#include <iostream>

using namespace std;

#define TIMER_TYPE_HELLO 1
#define TIMER_TYPE_ENTRY 2
#define MESSAGE_BUFFER_SIZE 32 * 1024
#define IPV4_ADDR_LEN 4

#define NO_ENTRY 0
#define HEARD 1
#define SYMMETRIC 2

#define LIVE_TIME 6000

#define	BROADCAST_ADDR	inet_addr("255.255.255.255")

typedef void (*timer_hfunc_t) (void *);

struct timerData {
	int				type;
	u_int32_t		data;

    //other contain, here we use this point to present a InterfaceEntry
    void *  contain;
};

struct controlData {
    uint32_t src;
    int ttl;
};

#endif