/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

void	rtable_entry::initMyRtableEntry()
{

	dest_ip = g_my_ip;
	dest_seq_num = localSeqNum;
	interface = 0;
	hop_cnt = 0;
	last_hop_cnt = 0;
	next_hop = g_my_ip;
	lifetime = getcurrtime()+MY_ROUTE_TIMEOUT;
	routing_flags = ACTIVE_ROUTE;
}


void	rtable_entry::createEntry(u_int32_t dest, u_int32_t seqNum, u_int8_t hopCnt, u_int32_t nhop, u_int64_t time, u_int32_t rFlag)
{
	dest_ip = dest;
	dest_seq_num = seqNum;
	interface = 0;
	hop_cnt = hopCnt;
	last_hop_cnt = 0;
	next_hop = nhop;
	lifetime = time;
	routing_flags = rFlag;
}

u_int32_t rtable_entry::getPrecCnt()
{
#ifdef DEBUG
	cout << "rtable_entry::getPrecCnt: entered " << endl;
	cout << "rtable_entry::getPrecCnt: precursor size is " << precursors.size() << endl;
#endif

	return precursors.size();
}

void	rtable_entry::addToPrecursor(u_int32_t precur)
{
#ifdef DEBUG
	cout << "rtable_entry: addToPrecursor: prec addr is " << getDotIP(precur) << endl;
	cout << "rtable_entry: addToPrecursor: entry addr is " << getDotIP(dest_ip) << endl;
#endif

	list<u_int32_t>::iterator	iter;

	iter = precursors.begin();

	while(iter != precursors.end())
	{
		if(*(iter) == precur)
		{
#ifdef DEBUG
			cout << "rtable_entry: addToPrecursor: already in the list " << endl;
#endif
			return;
		}
		iter++;
	}

#ifdef DEBUG
	cout << "rtable_entry: addToPrecursor: insert in the list " << endl;
#endif
	precursors.push_back(precur);
	return;
}


void	rtable_entry::getPrecAddr(u_int32_t *precAddr)
{
	list<u_int32_t>::iterator	iter;

	iter = precursors.begin();
	*precAddr = *(iter);
	return ;
}
