/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef RREQ_H
#define RREQ_H


class RREQ{

	u_char type:8;
	u_char J:1,
	R:1,
    	G:1,
    	reserved1:5;
    	u_char reserved2:8;
 	u_int8_t hop_cnt; 
 	u_int32_t rreq_id;
  	u_int32_t dest_ip;
  	u_int32_t dest_seq_num;
  	u_int32_t src_ip;
  	u_int32_t src_seq_num;

	public:


	RREQ(){}
	RREQ(RREQ *rreqOb);

	u_int32_t	getDestIP(){ return (dest_ip); }
	u_int32_t	getDestSeqNum(){ return (dest_seq_num); }
	u_int32_t	getSrcIP() { return (src_ip); }
	int			getG() { 
						if ( G == 1)
							return 1;
						else 
							return 0;
						}
	u_int32_t	getSrcSeqNum(){ return (src_seq_num); }
	u_int8_t	getHopCnt(){return (hop_cnt); }
	u_int32_t	getRreqId() { return (rreq_id);}

	void	setRreqId(u_int32_t id) { rreq_id = (id);}
	void	setDestSeqNum(u_int32_t seq) { dest_seq_num = (seq); }
	void	setSrcSeqNum(u_int32_t seq) { src_seq_num = (seq); }

	int		createRREQ(u_int32_t dst, u_int8_t flags);
	int		recvRREQ(u_int32_t prevHop);
};

#endif
