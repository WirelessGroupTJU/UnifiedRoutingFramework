/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

RREQ::RREQ(RREQ *rreqOb)
{
	type = rreqOb->type;
	J=0;
	R=0;
	G=rreqOb->G;
	reserved1 = 0;
	reserved2 = 0;
	hop_cnt = rreqOb->hop_cnt;
	rreq_id = rreqOb->rreq_id;	
	dest_ip = rreqOb->dest_ip;
	dest_seq_num = rreqOb->dest_seq_num;
	src_ip = rreqOb->src_ip;
	src_seq_num = rreqOb->src_seq_num;

}
int	RREQ::createRREQ(u_int32_t dst, u_int8_t flags)
{
	type = AODV_RREQ;
	J=0;
	R=0;
	if(flags == 1)
		G=(1);
	else 
		G=0;
	reserved1 = 0;
	reserved2 = 0;
	hop_cnt = 0;
	rreq_id = (++rreqId);	

	dest_ip = (dst);
	dest_seq_num = (rtable.getDestSeqNum(dst));
	src_ip = (g_my_ip);
	src_seq_num = (++localSeqNum);

	return 0;
}


int	RREQ::recvRREQ(u_int32_t prevHop)
{

	rtable_entry	*rtEntry=NULL;
	u_int64_t		currtime = getcurrtime();
	u_int32_t		seqNum =0;

#ifdef DEBUG
	cout << "rreq: recvRREQ: entered " << endl;
#endif

	/* ignore all RREQs received from nodes in the blacklist set */
	if(black_list.inList(prevHop))
	{
#ifdef DEBUG
		cout << "rreq: recvRREQ: RREQ received from a blacklist node, ignore it " << endl;
#endif
		return 0;
	}

	/* check for the local source ip */
	if((src_ip) == g_my_ip)
	{
#ifdef DEBUG
		cout << "Ignore RREQ from local node " << endl;
#endif
		return 0;
	}

	/* update routing entry for the prev hop node */
	/* pass the source ip and seq number */
	if(src_ip == prevHop)
		seqNum = src_seq_num;

	rtable.neighborUpdate(prevHop,src_ip,seqNum,(currtime + ALLOWED_HELLO_LOSS*HELLO_INTERVAL));

	/* check to see if RREQ is already processed */
	if(fw_rreqL.inList((src_ip), (rreq_id)))
	{
#ifdef DEBUG
		cout << "RREQ is already processed, ignore the received one " << endl;
#endif
		return 0;
	}

	/* enter RREQ in the list */
	fw_rreqL.addToList((src_ip), (rreq_id));

	/* create a reverse route */
	if(src_ip != prevHop)
		rtable.reverseRouteUpdate(prevHop, this);

	if((dest_ip) == g_my_ip)
	{
		/* rreq is for the local node - generate a rrep */
		/* update the local node's sequence number to the max of
		 * received and stored sequence number */
#ifdef DEBUG
		cout << "rreq: recvRREQ: rreq for local node " << endl;
		cout << "rreq: recvRREQ: recv dest seq num is" << dest_seq_num << endl;
#endif
		/* update the local node seq number */
		if(dest_seq_num > localSeqNum)
			localSeqNum = dest_seq_num;
		return GEN_RREP;
	}
	else
	{
		rtEntry = rtable.findValidRoute((dest_ip),(dest_seq_num));
		if(rtEntry != NULL)
		{
			/* there exist a valid route for the dest, generate a RREP */
			/* RREP generated at the intermediate node */
#ifdef DEBUG
			cout << "rreq: recvRREQ: valid route exist, send RREP " << endl;
#endif
			return GEN_RREP;
		}
		else
		{
			/* forward the RREQ */
#ifdef DEBUG
			cout << "rreq: recvRREQ: no valid route exist, forward RREP " << endl;
#endif
			rtEntry = rtable.findRoute((dest_ip));
			if(rtEntry != NULL)
			{
				/* here do not update the seq number of routing table 
				 * entry if it is smaller than the received seq num
				 * in the RREQ */
				if((dest_seq_num) < rtEntry->getDestSeqNum())
					dest_seq_num = (rtEntry->getDestSeqNum());
			}
			hop_cnt = (hop_cnt+1);
			return FWD_RREQ;
		}
	}
}

