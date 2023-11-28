/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef RREP_H
#define RREP_H

#include "rreq.h"

class RREP{

	
	u_char type:8;
  	u_char R:1,
	A:1,
	reserved1:6;
	u_char reserved2:3,
	prefix_size:5;
  	u_int8_t hop_cnt; 
  	u_int32_t dest_ip;
  	u_int32_t dest_seq_num;
  	u_int32_t src_ip;
  	u_int32_t lifetime;


	public:

	RREP(){}
	RREP(RREP *rrepOb);
	RREP(u_int32_t dst, u_int32_t seqNum, u_int32_t src, u_int8_t hopCnt, u_int32_t lifeTime);

	u_int8_t	getHopCnt() { return (hop_cnt); }
	u_int32_t	getDestIP() { return (dest_ip); }
	u_int32_t	getDestSeqNum() { return (dest_seq_num); }
	u_int32_t	getSrcIP() { return (src_ip); }
	u_int32_t	getLifeTime() { return (lifetime); }
	int			getA();

	void	createRREP( u_int32_t prevHop, RREQ *rreqOb);
	void    createGRREP(u_int32_t org_rreq_src, u_int32_t org_rreq_dst, u_int32_t seqNum);

	int		recvRREP(u_int32_t prevHop);	
};


#endif
