/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <signal.h>
#include <net/route.h>
#include <netinet/in.h>
#include <pthread.h>
#include <math.h>


#include <csignal>
#include <iostream>
#include <list>
#include <string>
#include <cstring>
#include <map>
#include <queue>

#include "timer.h"
#include "timerQueue.h"
#include "sendData.h"
#include "main.h"
#include "rtable_entry.h"
#include "routingTable.h"
#include "rreq.h"
#include "fwRreqList.h"
#include "rreqPendingList.h"
#include "rrep.h"
#include "rerr.h"
#include "rrepAck.h"
#include "localRepair.h"
#include "blacklist.h"
#include "aodvSocket.h"
#include "const.h"
#include "utils.h"
#include "parameters.h"
#include "aodv.h"

//我们的库函数
// #include "../ASL2.0/api.h"
#include "../lib/FO/fo.h"
#include "../lib/COMMON/la_message.h"
#include "../lib/ODS/ods_userspace_netlink.h"


#define SERV_PORT 12001

#endif


