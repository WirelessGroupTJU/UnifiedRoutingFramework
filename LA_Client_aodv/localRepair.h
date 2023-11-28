/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/

#ifndef _LOCAL_REPAIR_H_
#define _LOCAL_REPAIR_H_

class local_repair_entry{
	u_int32_t	dest_ip;

	public:

	u_int32_t	getDestIP() { return dest_ip;}
	void	setDestIP(u_int32_t dst) { dest_ip = dst; }
};

/* this contains the list of nodes which can be repaired locally,
 * every entry contains the destination IP of the locally repairable node */
class localRepair{

	list<local_repair_entry>	local_repair_list;

	public:

	void	addToList(u_int32_t ip);
	void	deleteFromList(list<local_repair_entry>::iterator iter);
	void	deleteFromList(u_int32_t dest);
	list<local_repair_entry>::iterator findInList(u_int32_t dst_ip);

};
#endif
