/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/

#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include "rtable_entry.h"
#include "rreq.h"
#include "rrep.h"
#include "rerr.h"

class routingTable{


	map<u_int32_t,rtable_entry>	rTableMap; /* map to store the routing table */

	public:

	void	insertRoute(rtable_entry rtEntry);
	void 	deleteRoute(rtable_entry rtEntry);
	void	init_rTable();

	u_int32_t	getDestSeqNum(u_int32_t dest);
	u_int8_t	getTTL(u_int32_t dst);

	void		updateDestSeqNum(u_int32_t dest, u_int32_t seqNum);

	bool		isRouteValid(map<u_int32_t,rtable_entry>::iterator iter);
	void		neighborUpdate(u_int32_t neighbor, u_int32_t src, u_int32_t seqNum, u_int64_t life_time);
	void		rebootNeighborUpdate(u_int32_t neighbor, u_int32_t seqNum, u_int64_t life_time);

	rtable_entry 	* findRoute(u_int32_t dest);
	map<u_int32_t,rtable_entry>::iterator	findRouteIter(u_int32_t dest);

	void		reverseRouteUpdate(u_int32_t nhop, RREQ *rreqOb);
	void		rebootReverseRouteUpdate(u_int32_t nhop, RREQ *rreqOb);
	void		forwardRouteUpdate(u_int32_t nhop, RREP *rrepOb);
	void		rebootForwardRouteUpdate(u_int32_t nhop, RREP *rrepOb);
	rtable_entry 	* findValidRoute(u_int32_t dest, u_int32_t destSeqNum);
	rtable_entry 	* findActiveRoute(u_int32_t dest);

	void		setLifeTime(u_int32_t dst_ip, u_int64_t lifetime);
	void		setDestSeqNum(u_int32_t	dst, u_int32_t seqNum);

	void		refreshEntries();
	int			del_kroute( map<u_int32_t,rtable_entry>::iterator iter );
	int			add_kroute( map<u_int32_t,rtable_entry>::iterator iter );
	struct rtentry *gen_krtentry( map<u_int32_t,rtable_entry>::iterator iter );

	void		deleteKernelRoute(u_int32_t	dst);
	void		neighbor_timeout_handler(void *data);
	void		addAllValidRoutes();
	void		addRoute(rtable_entry rtEntry);


};

#endif
