/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _RREQ_PENDING_LIST_H
#define _RREQ_PENDING_LIST_H

#include "rreq.h"

class rreq_list_entry{

	RREQ	        rreqOb;
	u_int32_t		retries;  /* number of times rreq has been sent with 
					    * ttl = TTL_THRESHOLD */
	int				ttl;
	//u_int32_t		lifetime;
	
	public:

	int			getTTL() { return ttl; }
	u_int32_t	getRetries() { return retries; }
	RREQ		*getRreqOb() { return &rreqOb;}

	void		setTTL(int tl) { ttl = tl; }
	void		setRetries(u_int32_t retry) { retries = retry; }
	void		setRreqId(u_int32_t id) { rreqOb.setRreqId(id); }
	void		setDestSeqNum(u_int32_t seq) { rreqOb.setDestSeqNum(seq); }
	void		setSrcSeqNum(u_int32_t seq) { rreqOb.setSrcSeqNum(seq); }
	friend class rreqPendingList;
};

/* this contains a list to store all the rreqs for which a rrep is being awaited */
class rreqPendingList{

	list<rreq_list_entry>	rreq_list;

	public:

	int	addToList( RREQ rOb, u_int32_t retry_val, int ttl_val);
	int	deleteFromList(u_int32_t dst);

	rreq_list_entry	*inList(u_int32_t dest, u_int32_t rreqId);
	rreq_list_entry	*inList(u_int32_t dest);
};

#endif
