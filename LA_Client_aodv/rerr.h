/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef RERR_H
#define RERR_H

#include "const.h"

class unrchDest {

	u_int32_t	dest_ip;
	u_int32_t	dest_seq_num;

	public:

	u_int32_t	getDestIP(){ return (dest_ip); }
	u_int32_t	getDestSeqNum() { return (dest_seq_num); }

	void	setDestIP(u_int32_t dst) { dest_ip = (dst); }
	void	setDestSeqNum(u_int32_t seqNum) { dest_seq_num = (seqNum); }
};

class RERR{

 	u_int8_t type;
  	u_int8_t N:1,
	reserved1:7;
  	u_int8_t reserved2;
  	u_int8_t dest_cnt;
	list<unrchDest>	unreachable_dest;

	public:

	RERR()
	{
		type = AODV_RERR;
		N=0;
		reserved1 = 0;
		reserved2 = 0;
		dest_cnt = 0;
	}

	u_int8_t	getDestCnt() { return (dest_cnt); }
	u_int32_t	getUnrchDest();
	u_int32_t	getUnrchDestSeqNum();
	u_int8_t	getType() { return type; }
	int			getN();

	void	setDestCnt(u_int8_t cnt) { dest_cnt = (cnt); }
	void	setUnrchDest(u_int32_t dst, u_int32_t seq);
	void	setNFlag(u_int8_t val) { 
								if( val == 1)
									N = 1;
   								else
									N = 0;	}

	void	copyIntoBuf(char *buf);
	void	createRERR(char *buf);
	bool	createNewRERR(RERR *rerrOb, u_int32_t prevHop,  u_int32_t *addr);
	
	void	updateInvalidEntries();
	void	updateValidEntries(u_int32_t prevHop);

};

#endif
