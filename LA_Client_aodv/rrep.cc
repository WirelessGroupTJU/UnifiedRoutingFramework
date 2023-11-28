/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

RREP::RREP(RREP * rrepOb)
{
	type = rrepOb->type;
	R=0;
	A=rrepOb->A;
	reserved1=0;
	reserved2=0;
	prefix_size=rrepOb->prefix_size;
	hop_cnt = rrepOb->hop_cnt;
	dest_ip = rrepOb->dest_ip;
	dest_seq_num = rrepOb->dest_seq_num;
	src_ip = rrepOb->src_ip;
	lifetime = rrepOb->lifetime;

}

RREP::RREP(u_int32_t dst, u_int32_t seqNum, u_int32_t srcIP, u_int8_t hopCnt, u_int32_t lTime)
{
	type = AODV_RREP;
	R = 0;
	A = 0;
	reserved1=0;
	reserved2=0;
	prefix_size=0;
	hop_cnt = (hopCnt);
	dest_ip = (dst);
	dest_seq_num = (seqNum);
	src_ip = (srcIP);
	lifetime = (lTime);

}

int		RREP::getA()
{
	if( A == 1)
		return 1;
	else 
		return 0;
}

void	RREP::createRREP(u_int32_t prevHop, RREQ *rreqOb)
{
	rtable_entry	*rtEntryDst, *rtEntrySrc;

	type = AODV_RREP;
	R=0;
	if(rrepAckFlag)
		A=(1);
	else
		A=0;
	reserved1=0;
	reserved2=0;
	prefix_size=0;
	
#ifdef DEBUG
	cout << "RREP: createRREP: entered " << endl;
#endif

	if(rreqOb->getDestIP() == g_my_ip)
	{
#ifdef DEBUG
		cout << "RREP: createRREP: local node case " << endl;
#endif
		dest_seq_num = localSeqNum; 
		hop_cnt = 0;
		lifetime = (MY_ROUTE_TIMEOUT);
	}
	else
	{
		/* intermediate node case */
		/* also update the precursor list for both the src and destination entries */
#ifdef DEBUG
		cout << "RREP: createRREP: intermediate node case " << endl;
#endif
		rtEntryDst = rtable.findActiveRoute(rreqOb->getDestIP());
		if(rtEntryDst != NULL)
		{
			dest_seq_num = (rtEntryDst->getDestSeqNum());
			hop_cnt = (rtEntryDst->getHopCnt());

#ifdef DEBUG
			cout << "RREP: createRREP: hop count is " << (int)rtEntryDst->getHopCnt() << endl;
			cout << "RREP: createRREP: hop count is " << (int)hop_cnt << endl;
			cout << "RREP: createRREP: add prec entry " << endl;
#endif
			rtEntryDst->addToPrecursor(prevHop);
			lifetime = (u_int32_t)(rtEntryDst->getLifeTime()-getcurrtime());
			//lifetime = (u_int32_t)(15*ACTIVE_ROUTE_TIMEOUT);

#ifdef DEBUG
			cout << "RREP: createRREP: lifetime is " << lifetime << endl;
#endif
			rtEntrySrc = rtable.findActiveRoute(rreqOb->getSrcIP());
			if(rtEntrySrc != NULL)
			{
#ifdef DEBUG
				cout << "RREP: createRREP: add to src prec list " << endl;
#endif
				rtEntrySrc->addToPrecursor(rtEntryDst->getNextHop());
			}
		}
	}	
	
	dest_ip = (rreqOb->getDestIP());
	src_ip = (rreqOb->getSrcIP());

}
 
void	RREP::createGRREP(u_int32_t org_rreq_src, u_int32_t org_rreq_dst, u_int32_t seqNum)
{
	rtable_entry	*rtEntry;

	type = AODV_RREP;
	R=0;
	if(rrepAckFlag)
		A=(1);
	else
		A=0;
	reserved1=0;
	reserved2=0;
	prefix_size=0;

#ifdef DEBUG
	cout << "RREP: createGRREP: entered " << endl;
#endif

	rtEntry = rtable.findActiveRoute(org_rreq_src);
	if(rtEntry != NULL)
	{
		hop_cnt = (rtEntry->getHopCnt());
		lifetime = (u_int32_t)(rtEntry->getLifeTime()-getcurrtime());
#ifdef DEBUG
		cout << "RREP: createGRREP: hop cnt is " << (int)hop_cnt << endl;
#endif

	}

	dest_ip = (org_rreq_src);
	dest_seq_num = (seqNum);
	src_ip = (org_rreq_dst);
}

int		RREP::recvRREP(u_int32_t prevHop)
{
	rtable_entry	*rtEntryDst, *rtEntrySrc;
	u_int64_t		currtime = getcurrtime();
	u_int32_t		seqNum=0;

	/* update routing entry for the prev hop node */
	/* pass the source ip and seq number */
	if(dest_ip == prevHop)
		seqNum = dest_seq_num;

	rtable.neighborUpdate(prevHop,src_ip,seqNum,(currtime+ALLOWED_HELLO_LOSS*HELLO_INTERVAL));
	
	if(dest_ip != prevHop)
		rtable.forwardRouteUpdate(prevHop, this);

#ifdef DEBUG
	cout << "RREP: recvRREP: dest seq num is" << dest_seq_num << endl;
#endif

	/* if the RREP is not for the local node and it was
	 * not a hello message */
	if(((src_ip) != g_my_ip) && (src_ip !=0))
	{
		/* need to forward the rrep */
		/* update the precursor list here */
		rtEntryDst = rtable.findActiveRoute((dest_ip));
		rtEntrySrc = rtable.findActiveRoute((src_ip));
		if(rtEntryDst != NULL)
		{
			if(rtEntrySrc != NULL)
				rtEntryDst->addToPrecursor(rtEntrySrc->getNextHop());
		}
		hop_cnt = (hop_cnt+1);

		/* reset the rrep ack flag at this node */
		if(rrepAckFlag)
			A=1;
		else
			A=0;

		return SEND_RREP;
	}

	return 0;
}
