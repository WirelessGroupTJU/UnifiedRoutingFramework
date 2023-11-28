/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

/* find an entry in the local repair list */
list<local_repair_entry>::iterator	localRepair::findInList(u_int32_t dst)
{
	list<local_repair_entry>::iterator	iter;

	iter = local_repair_list.begin();

	while(iter != local_repair_list.end())
	{
		if(iter->getDestIP() == dst)
			return iter;
		iter++;
	}
	return (list<local_repair_entry>::iterator)NULL;
}

/* add entries to the local repair list */
void	localRepair::addToList(u_int32_t dst)
{
#ifdef DEBUG
	cout << "localRepair: addToList: entered " << endl;
#endif

	list<local_repair_entry>::iterator	iter;

	iter = findInList(dst);
	if(iter != (std::list<local_repair_entry>::iterator)NULL)
	{
#ifdef DEBUG
		cout << "localRepair: addToList: entry already exist " << endl;
#endif
		return;
	}

	local_repair_entry	lpEntry;

	lpEntry.setDestIP(dst);
	local_repair_list.push_back(lpEntry);
	return;
}

/* delete entry from the local repair list */
void	localRepair::deleteFromList(list<local_repair_entry>::iterator iter)
{

#ifdef DEBUG
	cout << "localRepair: deleteFromList: entered " << endl;
#endif

	local_repair_list.erase(iter);
	return;
}


/* delete entry from the local repair list */
void	localRepair::deleteFromList(u_int32_t dst)
{

#ifdef DEBUG
	cout << "localRepair: deleteFromList: entered " << endl;
#endif

	list<local_repair_entry>::iterator	iter;

	iter = findInList(dst);
	if(iter != (std::list<local_repair_entry>::iterator)NULL)
	{
		local_repair_list.erase(iter);
	}
	return;
}

