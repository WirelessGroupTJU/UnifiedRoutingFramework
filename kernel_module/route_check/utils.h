/* Some utility functions borrowed from utils.c of  NIST's kernel-AODV
* implementation by Luke Klein-Berndt.*/

#ifndef UTIL_H
#define UTIL_H

#include <linux/module.h>
#include <linux/socket.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/ctype.h>
#include <linux/byteorder/generic.h>

#include "module_main.h"



int inet_aton(const char *cp, __u32 *addr);
char *inet_ntoa(__u32 ina);

u_int64_t getcurrtime(void);

#endif 

















