/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _CONST_H_
#define _CONST_H_

/* constant definitions used in the implementation */

#define	MAX_HANDLERS	8

#define GEN_RREP	1
#define	FWD_RREQ	2
#define	SEND_RREP   3

#define REBOOT_TIMER 	4
#define	RREQ_TIMER	 	5
#define	HELLO_TIMER	 	6
#define	LR_TIMER	 	7	
#define	PERIODIC_TIMER	8 
#define	RREP_ACK_TIMER	9 
#define	ACTIVE_ROUTE_TIMER 10

#define	BROADCAST_ADDR	inet_addr("255.255.255.255")

#define		MAX(a,b) (((a)>(b))?(a):(b))

/* AODV message types */
#define AODV_RREQ		1
#define	AODV_RREP		2
#define	AODV_RERR		3
#define AODV_RREP_ACK	4

#define GRREP_FLAG		1

#define	AODV_PORT		654

/* routing flags */

#define	BROKEN_ROUTE		1
#define	LOCAL_REPAIR_ROUTE 	2
#define	ACTIVE_ROUTE		3
#define INVALID_ROUTE		4


#endif
