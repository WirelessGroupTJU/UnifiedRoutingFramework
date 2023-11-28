/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef RTABLE_ENTRY_H
#define RTABLE_ENTRY_H

#include "utils.h"

class rtable_entry{

 	u_int32_t dest_ip;
  	u_int32_t dest_seq_num;
  	unsigned int interface;  
  	u_int8_t hop_cnt;
  	u_int8_t last_hop_cnt;  
  	u_int32_t next_hop;
	/* list of precursor node ip addresses */
	list<u_int32_t>	precursors;
  	u_int64_t lifetime;
  	u_int32_t routing_flags;

	public:

	void		initMyRtableEntry();

	void		createEntry(u_int32_t dest, u_int32_t seqNum, u_int8_t hopCnt,
				u_int32_t next_hop, u_int64_t lifetime, u_int32_t rFlag);

	u_int32_t	getRFlags() { return routing_flags; }
	u_int32_t	getDestSeqNum() { return dest_seq_num; }
	u_int8_t	getHopCnt() { return hop_cnt; }
	u_int8_t	getLastHopCnt() { return last_hop_cnt; }
	u_int64_t	getLifeTime() { return lifetime; }
	u_int32_t	getNextHop() { return next_hop; }
	u_int32_t	getDestIP() { return dest_ip; }
	u_int32_t	getPrecCnt();

	void		getPrecAddr(u_int32_t *precAddr);


	void		setRFlags(u_int32_t flag) { routing_flags = flag; }
	void		setDestSeqNum(u_int32_t seqNum) { dest_seq_num = seqNum;}
	void		setNextHop(u_int32_t nhop) { next_hop = nhop;}
	void		setHopCnt(u_int8_t	hopCnt) { hop_cnt = hopCnt; }
	void		setLastHopCnt(u_int8_t	lastHopCnt) { last_hop_cnt = lastHopCnt; }
	void		setLifeTime(u_int64_t time) { lifetime = time;}
	void		addToPrecursor(u_int32_t precur);
};


#endif
