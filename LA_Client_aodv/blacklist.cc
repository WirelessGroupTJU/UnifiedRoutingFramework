/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

/* add destination to the black list */
void	blacklist::addToList(u_int32_t dst, u_int64_t lTime)
{

#ifdef DEBUG
	cout << "blacklist: addToList: entered" << endl;
#endif

	if(inList(dst))
	{
#ifdef DEBUG
		cout << "blacklist: addToList: already in the list" << endl;
#endif
		return;
	}

	blacklist_entry		bEntry;

	bEntry.setDestIP(dst);
	bEntry.setLifeTime(getcurrtime()+lTime);

	bList.push_back(bEntry);
	return;
}

/* check if the destination is already in the list */
bool	blacklist::inList(u_int32_t dst)
{
	list<blacklist_entry>::iterator	iter;

	iter = bList.begin();

	while(iter != bList.end())
	{
		if(iter->getDestIP() == dst)
			return true;
	}

	return false;
}

/* expire stale entries from the blacklist */
void	blacklist::updateEntries()
{
	list<blacklist_entry>::iterator	iter, newIter;
	u_int64_t	currtime = getcurrtime();

#ifdef DEBUG
	cout << "blacklist: updateEntries: entered" << endl;
#endif

	iter = bList.begin();

	while(iter != bList.end())
	{
		iter++;
		newIter = iter;
		iter--;

		if(iter->getLifeTime() <= currtime)
		{
#ifdef DEBUG
			cout << "blacklist: updateEntries: remove from the list" << endl;
#endif
			bList.erase(iter);
		}
		iter = newIter;
	}
	return;
}
