/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _EXTERN_DEC_H_
#define _EXTERN_DEC_H_

/* Global Variables */
extern 	aodv	aodvOb;

extern	u_int32_t	rreqId;
extern	u_int32_t	localSeqNum;
extern	u_int32_t	g_my_ip;
extern	int			asl_sock;

/*zyp*/
extern	int			la_sock;			

extern	int			ksock;

extern	routingTable 	rtable;
extern	rreqPendingList	rreqL;
extern	fwRreqList		fw_rreqL;
extern	localRepair	local_repair;
extern	timerQueue	timer_Q;
extern  blacklist	black_list;

extern	char		*recvBuffer;
extern	char		*sendBuffer;
extern	char 		interface[10];

extern	bool  	helloSendFlag;
extern	bool 	expandingRingSearch;
extern	bool	rrepAckFlag;
extern	bool	duringReboot;
extern	bool	needToSendHello;
extern	bool	localRepairFlag;

#endif
