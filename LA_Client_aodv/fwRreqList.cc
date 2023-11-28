/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/



#include "common.h"
#include "externDec.h"

void	fwRreqList::addToList(u_int32_t src, u_int32_t rreqId)
{
	fw_rreq_entry	entry;

#ifdef DEBUG
	cout << "fwRreqList: addToList: entered " << endl;
#endif

	if( inList(src, rreqId))
	{
#ifdef DEBUG
		cout << "fwRreqList: addToList: entry already exist " << endl;
#endif
		return;
	}

	entry.setSrcIP(src);
	entry.setRreqId(rreqId);
	entry.setLifeTime((getcurrtime()+PATH_TRAVERSAL_TIME));

	fw_rreq_list.push_back(entry);
	return;
}

bool	fwRreqList::inList(u_int32_t src, u_int32_t rreqId)
{
	list<fw_rreq_entry>::iterator	iter;

#ifdef DEBUG
	cout << "fwRreqList: inList: entered " << endl;
#endif

	iter = fw_rreq_list.begin();
	while(iter != fw_rreq_list.end())
	{
		if((iter->getSrcIP() == src) && (iter->getRreqId() == rreqId))
		{
			return true;
		}
		++iter;
	}

	return false;
}

/* remove expired entries from the fwrreq list */
void	fwRreqList::updateEntries()
{
	list<fw_rreq_entry>::iterator	iter, temp_iter;

#ifdef DEBUG
	cout << "fwRreqList: updateEntries: entered" << endl;
#endif
	iter = fw_rreq_list.begin();
	while(iter != fw_rreq_list.end())
	{
		iter++;
		temp_iter = iter;
		iter--;

		if(iter->getLifeTime() <= getcurrtime())
		{
			fw_rreq_list.erase(iter);
		}
		iter = temp_iter;
	}
	return;

}
