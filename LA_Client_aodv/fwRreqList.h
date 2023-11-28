/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/

#ifndef _RREQ_LIST_H
#define _RREQ_LIST_H

class fw_rreq_entry {
	u_int32_t	src_ip;
	u_int32_t	rreq_id;
	u_int64_t	lifetime;

	public:

	u_int32_t	getSrcIP() { return src_ip;}
	u_int32_t	getRreqId() { return rreq_id;}
	u_int64_t	getLifeTime() { return lifetime;}

	void		setSrcIP(u_int32_t src) { src_ip = src;}
	void		setRreqId(u_int32_t rreqId) { rreq_id = rreqId;}
	void 		setLifeTime(u_int64_t lTime) {lifetime = lTime;}

};

/* this list stores rreqs which have been processed recently by the local node,
 * to avoid reprocessing them */

class fwRreqList{
	list<fw_rreq_entry>		fw_rreq_list;

	public:

	bool	inList(u_int32_t src, u_int32_t rreqId);
	void	addToList(u_int32_t src, u_int32_t rreqId);
	void 	updateEntries();

};


#endif
