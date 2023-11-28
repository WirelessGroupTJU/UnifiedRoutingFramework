/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

/* check if the rreq for the dst is in the pending list of rreqs */
rreq_list_entry	*rreqPendingList::inList(u_int32_t dst, u_int32_t rreqId)
{

	list<rreq_list_entry>::iterator	iter;

	iter = rreq_list.begin();
	while(iter != rreq_list.end())
	{
		if((iter->rreqOb.getDestIP() == dst) && (iter->rreqOb.getRreqId()))
		{
			return &(*iter);
		}
		++iter;
	}
	return NULL;
}


rreq_list_entry	*rreqPendingList::inList(u_int32_t dst)
{

#ifdef DEBUG
	cout << "rreqPendingList: inList: entered " << endl;
#endif

	list<rreq_list_entry>::iterator	iter;

	iter = rreq_list.begin();
	while(iter != rreq_list.end())
	{
		if((iter->rreqOb.getDestIP() == dst))
		{
#ifdef DEBUG
			cout << "rreqPendingList: found in list " << endl;
#endif
			return &(*iter);
		}
		++iter;
	}
	return NULL;
}

int	rreqPendingList::addToList(RREQ rOb, u_int32_t retry_val, int ttl_val)
{
	rreq_list_entry	entry;

#ifdef DEBUG
	cout << "rreqPendingList: addToList: entered " << endl;
	cout << "rreqPendingList: addToList: dest ip is " << getDotIP(rOb.getDestIP()) << endl;
#endif

	entry.rreqOb = rOb;
	entry.retries = retry_val;
	entry.ttl = ttl_val;

	rreq_list.push_back(entry);

	return 0;
}

int	rreqPendingList::deleteFromList(u_int32_t dest)
{
	list<rreq_list_entry>::iterator	iter;

	iter = rreq_list.begin();
	while(iter != rreq_list.end())
	{
		if(iter->rreqOb.getDestIP() == dest)
		{
			rreq_list.erase(iter);
			break;
		}
		++iter;
	}

	return 0;
}
