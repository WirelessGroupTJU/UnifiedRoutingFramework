/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

void		neighborTimeoutHandler(void *data);


void		routingTable::init_rTable()
{
#ifdef DEBUG
	cout << "init_rTable: entered" << endl;
#endif
	rtable_entry	rtEntry;
	rtEntry.initMyRtableEntry();
	insertRoute(rtEntry);

}

void		routingTable::addRoute(rtable_entry rtEntry)
{
	rTableMap.insert(pair<u_int32_t,rtable_entry>(rtEntry.getDestIP(),rtEntry));
	return;
}

void		routingTable::insertRoute(rtable_entry rtEntry)
{
	rTableMap.insert(pair<u_int32_t,rtable_entry>(rtEntry.getDestIP(),rtEntry));
	/* insert this route in kernel also */
	map<u_int32_t,rtable_entry>::iterator	iter;

#ifdef DEBUG
	cout << "insertRoute: entered" << endl;
#endif
	iter = findRouteIter(rtEntry.getDestIP());
	add_kroute(iter);

}

void		routingTable::addAllValidRoutes()
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.begin();
	while(iter != rTableMap.end())
	{
		if(iter->second.getRFlags() == ACTIVE_ROUTE)
		{
			/* add this route to the kernel */
			add_kroute(iter);
		}
		iter++;
	}
	return;
}

u_int32_t	routingTable::getDestSeqNum(u_int32_t dst)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.find(dst);

	if(iter != rTableMap.end())
		return (iter->second.getDestSeqNum());
	else
		return 0;

}

void		routingTable::setDestSeqNum(u_int32_t	dst, u_int32_t seqNum)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.find(dst);
	if(iter != rTableMap.end())
	{
		iter->second.setDestSeqNum(seqNum);
	}
}

u_int8_t	routingTable::getTTL(u_int32_t dst)
{
	map<u_int32_t,rtable_entry>::iterator	iter;
	iter = rTableMap.find(dst);

	if(iter != rTableMap.end())
	{
		if(isRouteValid(iter))
			return iter->second.getHopCnt();
		else
			return iter->second.getLastHopCnt();
	}
	else
		return 0;

}

void		routingTable::setLifeTime(u_int32_t dst, u_int64_t lifetime)
{
	map<u_int32_t,rtable_entry>::iterator	iter;
	iter = rTableMap.find(dst);

	if(iter != rTableMap.end())
	{
		iter->second.setLifeTime(lifetime);
	}
	return;
}

bool	routingTable::isRouteValid(map<u_int32_t,rtable_entry>::iterator iter)
{
	if((iter->second.getHopCnt() != INFINITY) && (iter->second.getLifeTime() > getcurrtime()))
		return true;
	else 
		return false;

}

void	routingTable::updateDestSeqNum(u_int32_t dest, u_int32_t seqNum)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.find(dest);

	if(iter != rTableMap.end())
	{
		if(iter->second.getDestSeqNum() < seqNum)
			iter->second.setDestSeqNum(seqNum);
	}
	return;
}


rtable_entry*	routingTable::findRoute(u_int32_t dest)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.find(dest);
	if(iter != rTableMap.end())
		return &(iter->second);
	else
		return NULL;
}


map<u_int32_t,rtable_entry>::iterator	routingTable::findRouteIter(u_int32_t dest)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.find(dest);
	if(iter != rTableMap.end())
		return iter;
	else
		return (map<u_int32_t,rtable_entry>::iterator)NULL;

}	

/* add/update reverse route entry during reboot time */
void		routingTable::rebootReverseRouteUpdate(u_int32_t nhop, RREQ *rreqOb)
{
	u_int64_t	minLifeTime;
	map<u_int32_t,rtable_entry>::iterator	iter;

#ifdef DEBUG
	cout << "routingTable: rebootReverseRouteUpdate:\
	   	entered " << endl;
#endif

	iter = rTableMap.find(rreqOb->getSrcIP());
	if(iter != rTableMap.end())
	{
#ifdef DEBUG
		cout << "routingTable: rebootReverseRouteUpdate: route already exist " << endl;
#endif
		if(iter->second.getRFlags() == ACTIVE_ROUTE)
		{
#ifdef DEBUG
			cout << "routinTable: rebootReverseRouteUpdate: a valid route already exist " << endl;
#endif

			/* a route to the source already exist */
			if((rreqOb->getSrcSeqNum() > iter->second.getDestSeqNum()) ||
			 ((rreqOb->getSrcSeqNum() == iter->second.getDestSeqNum()) && 
			 ((rreqOb->getHopCnt()+1) < iter->second.getHopCnt())))
			{
				iter->second.setDestSeqNum(rreqOb->getSrcSeqNum());
				iter->second.setNextHop(nhop);
				iter->second.setHopCnt((rreqOb->getHopCnt()+1));
				minLifeTime = getcurrtime()+PATH_TRAVERSAL_TIME - 2*iter->second.getHopCnt()*NODE_TRAVERSAL_TIME;
				if(minLifeTime > iter->second.getLifeTime())
					iter->second.setLifeTime(minLifeTime);
			}
		}
		else
		{
#ifdef DEBUG
			cout << "routingTable: rebootReverseRouteUpdate: an invalid route exist " << endl;
#endif

			iter->second.setDestSeqNum(rreqOb->getSrcSeqNum());
			iter->second.setNextHop(nhop);
			iter->second.setHopCnt((rreqOb->getHopCnt()+1));
			iter->second.setRFlags(ACTIVE_ROUTE);

			minLifeTime = getcurrtime()+PATH_TRAVERSAL_TIME - 2*iter->second.getHopCnt()*NODE_TRAVERSAL_TIME;
			iter->second.setLifeTime(minLifeTime);
		}
	}
	else
	{
		rtable_entry	rtEntry;
		minLifeTime = getcurrtime()+PATH_TRAVERSAL_TIME - 2*(rreqOb->getHopCnt()+1)*NODE_TRAVERSAL_TIME;

		rtEntry.createEntry(rreqOb->getSrcIP(),rreqOb->getSrcSeqNum(),(rreqOb->getHopCnt()+1), nhop,minLifeTime, ACTIVE_ROUTE);
		addRoute(rtEntry);
	}
	return;
}

/* add/update reverse route entry*/
void		routingTable::reverseRouteUpdate(u_int32_t nhop, RREQ *rreqOb)
{
	u_int64_t	minLifeTime;
	u_int32_t	prevNextHop;
	bool		addFlag = false;
	map<u_int32_t,rtable_entry>::iterator	iter;

#ifdef DEBUG
	cout << "routingTable: reverseRouteUpdate: entered " << endl;
#endif

	iter = rTableMap.find(rreqOb->getSrcIP());
	if(iter != rTableMap.end())
	{
#ifdef DEBUG
		cout << "routingTable: reverseRouteUpdate: route already exist " << endl;
#endif
		if(iter->second.getRFlags() == ACTIVE_ROUTE)
		{
#ifdef DEBUG
			cout << "routinTable: reverseRouteUpdate: a valid route already exist " << endl;
#endif

			/* a route to the source already exist */
			if((rreqOb->getSrcSeqNum() > iter->second.getDestSeqNum()) ||
			 ((rreqOb->getSrcSeqNum() == iter->second.getDestSeqNum()) && 
			 ((rreqOb->getHopCnt()+1) < iter->second.getHopCnt())))
			{

#ifdef DEBUG
				cout << "routingTable: reverseRouteUpdate: update existing route " << endl;
#endif
				/* delete the old route from kernel if needed */
				prevNextHop = iter->second.getNextHop();
				if(((rreqOb->getHopCnt()+1) < iter->second.getHopCnt()) || (prevNextHop != nhop))
				{
					del_kroute(iter);
					addFlag = true;
				}

				iter->second.setDestSeqNum(rreqOb->getSrcSeqNum());
				iter->second.setNextHop(nhop);
				iter->second.setHopCnt((rreqOb->getHopCnt()+1));
				minLifeTime = getcurrtime()+PATH_TRAVERSAL_TIME - 2*iter->second.getHopCnt()*NODE_TRAVERSAL_TIME;
				if(minLifeTime > iter->second.getLifeTime())
					iter->second.setLifeTime(minLifeTime);

				/* add new route to the kernel */
				if( addFlag )
				{
					add_kroute(iter);
				}
			}
		}
		else
		{

#ifdef DEBUG
			cout << "routingTable: reverseRouteUpdate: an invalid route exist " << endl;
#endif

			iter->second.setDestSeqNum(rreqOb->getSrcSeqNum());
			iter->second.setNextHop(nhop);
			iter->second.setHopCnt((rreqOb->getHopCnt()+1));
			iter->second.setRFlags(ACTIVE_ROUTE);

			minLifeTime = getcurrtime()+PATH_TRAVERSAL_TIME - 2*iter->second.getHopCnt()*NODE_TRAVERSAL_TIME;
			iter->second.setLifeTime(minLifeTime);

			add_kroute(iter);
		}

	}
	else
	{
		/* reverse route needs to be created */
#ifdef DEBUG
		cout << "routingTable: reverseRouteUpdate: create reverse route " << endl;
#endif
		rtable_entry	rtEntry;
		minLifeTime = getcurrtime()+PATH_TRAVERSAL_TIME - 2*(rreqOb->getHopCnt()+1)*NODE_TRAVERSAL_TIME;
		rtEntry.createEntry(rreqOb->getSrcIP(),rreqOb->getSrcSeqNum(),(rreqOb->getHopCnt()+1), nhop,minLifeTime, ACTIVE_ROUTE);

		insertRoute(rtEntry);
	}

	/* now a valid reverse route has been created */
	//zyp
	LAMessage feedback_message;
	feedback_message.cmd = LA_ODS_REPLY;
	feedback_message.dst_address = rreqOb->getSrcIP();
	feedback_message.src_address = 0;
	feedback_message.protocol = 0;
	feedback_message.result = IP_FOUND;
	
	char send_buf[1024];
	int len = 0;
	len = feedback_message.generate_stream(send_buf, 1024);
	int send_ret = send(la_sock, send_buf, len, 0);
	// route_discovery_done(rreqOb->getSrcIP(), ASL_ROUTE_FOUND);

	/* also remove the rreq timer */
	int ret = timer_Q.removeRREQTimer((rreqOb->getSrcIP()));
	if(ret == LR_TIMER)
	{
		/* remove the dest from local repair list ..
		 * also see if a route err need to be generated */

		local_repair.deleteFromList(rreqOb->getSrcIP());

		rtable_entry	*rtEntry;
		rtEntry = findRoute(rreqOb->getSrcIP());

		if((rtEntry != NULL) && (rtEntry->getHopCnt() > rtEntry->getLastHopCnt()))
			aodvOb.generateRERR(rreqOb->getSrcIP(),1, false);
	}
	return;
}

/* add/update forward route entry during reboot */
void		routingTable::rebootForwardRouteUpdate(u_int32_t nhop, RREP *rrepOb)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

#ifdef DEBUG
	cout << "rebootForwardRouteUpdate: entered dest is" << getDotIP(rrepOb->getDestIP()) << endl;
#endif

	iter = rTableMap.find(rrepOb->getDestIP());
	if(iter != rTableMap.end())
	{

		if(iter->second.getRFlags() == ACTIVE_ROUTE)
		{
#ifdef DEBUG
			cout << "routingTable: rebootForwardRouteUpdate: an active route exist " << endl;
#endif
			if((rrepOb->getDestSeqNum() > iter->second.getDestSeqNum()) ||
				((rrepOb->getDestSeqNum() == iter->second.getDestSeqNum()) 
				&& ((rrepOb->getHopCnt()+1) < iter->second.getHopCnt())))
			{

				iter->second.setNextHop(nhop);
				iter->second.setHopCnt((rrepOb->getHopCnt()+1));
				iter->second.setDestSeqNum(rrepOb->getDestSeqNum());
				iter->second.setLifeTime(getcurrtime()+ rrepOb->getLifeTime());
			}
		}
		else
		{
			iter->second.setNextHop(nhop);
			iter->second.setHopCnt((rrepOb->getHopCnt()+1));
			iter->second.setDestSeqNum(rrepOb->getDestSeqNum());
			iter->second.setLifeTime(getcurrtime()+ rrepOb->getLifeTime());
			iter->second.setRFlags(ACTIVE_ROUTE);
		}
	}
	else
	{
		rtable_entry	rtEntry;
		rtEntry.createEntry(rrepOb->getDestIP(),rrepOb->getDestSeqNum(),(rrepOb->getHopCnt()+1), nhop, (getcurrtime()+rrepOb->getLifeTime()), ACTIVE_ROUTE);

		addRoute(rtEntry);
	}
	return;
}


/* add/update forward route entry*/
void		routingTable::forwardRouteUpdate(u_int32_t nhop, RREP *rrepOb)
{
	u_int32_t	prevNextHop;
	bool		addFlag = false;
	map<u_int32_t,rtable_entry>::iterator	iter;

#ifdef DEBUG
	cout << "forwardRouteUpdate: entered dest is" << getDotIP(rrepOb->getDestIP()) << endl;
#endif

	iter = rTableMap.find(rrepOb->getDestIP());
	if(iter != rTableMap.end())
	{
		/* an entry already exists */
#ifdef DEBUG
		cout  << "routingTable: forwardRouteUpdate: an entry exists " << endl;
		cout  << "routingTable: forwardRouteUpdate: rrep seq num" << rrepOb->getDestSeqNum() << endl;
		cout  << "routingTable: forwardRouteUpdate: rtable seq num " << iter->second.getDestSeqNum() << endl;
		cout  << "routinTable: forwardRouteUpdate: rtable hop cnt is " << (int)iter->second.getHopCnt() << endl;
#endif

		if(iter->second.getRFlags() == ACTIVE_ROUTE)
		{

#ifdef DEBUG
			cout << "routingTable: forwardRouteUpdate: an active route exist " << endl;
#endif

			if((rrepOb->getDestSeqNum() > iter->second.getDestSeqNum()) ||
				((rrepOb->getDestSeqNum() == iter->second.getDestSeqNum()) 
				&& ((rrepOb->getHopCnt()+1) < iter->second.getHopCnt())))
			{
				/* delete the old route from kernel if needed */
#ifdef DEBUG
				cout  << "routingTable: forwardRouteUpdate: update the existing entry " << endl;
#endif

				prevNextHop = iter->second.getNextHop();
				if(((rrepOb->getHopCnt()+1) < iter->second.getHopCnt()) || (prevNextHop != nhop))
				{
					del_kroute(iter);
					addFlag = true;
				}

				iter->second.setNextHop(nhop);
				iter->second.setHopCnt((rrepOb->getHopCnt()+1));
				iter->second.setDestSeqNum(rrepOb->getDestSeqNum());
				iter->second.setLifeTime(getcurrtime()+ rrepOb->getLifeTime());

				/* add new route to the kernel */
				if( addFlag )
				{
					add_kroute(iter);
				}
			}	
		}
		else
		{
#ifdef DEBUG
			cout << "routingTable: forwardRouteUpdate: an invalid route exist " << endl;
#endif

			iter->second.setNextHop(nhop);
			iter->second.setHopCnt((rrepOb->getHopCnt()+1));
			iter->second.setDestSeqNum(rrepOb->getDestSeqNum());
			iter->second.setLifeTime(getcurrtime()+ rrepOb->getLifeTime());
			iter->second.setRFlags(ACTIVE_ROUTE);

			add_kroute(iter);
		}
	}
	else
	{
		/* create a forward route entry */
		rtable_entry	rtEntry;

#ifdef DEBUG
		cout  << "routingTable: forwardRouteUpdate: an entry does not exist " << endl;
		cout  << "routingTable: forwardRouteUpdate: an entry does not exist Hop Cnt is " << (int)rrepOb->getHopCnt() << endl;
#endif

		rtEntry.createEntry(rrepOb->getDestIP(),rrepOb->getDestSeqNum(),(rrepOb->getHopCnt()+1), nhop, (getcurrtime()+rrepOb->getLifeTime()), ACTIVE_ROUTE);

		insertRoute(rtEntry);
#ifdef DEBUG
		cout  << "routingTable: forwardRouteUpdate: an entry does not exist Hop Cnt is " << (int)rtEntry.getHopCnt() << endl;
		cout  << "routingTable: forwardRouteUpdate: an entry does not exist life time is " << rrepOb->getLifeTime() << endl;
#endif
	}

	/* now a valid forward route has been created */
	//zyp
	LAMessage feedback_message;
	feedback_message.cmd = LA_ODS_REPLY;
	feedback_message.dst_address = rrepOb->getDestIP();
	feedback_message.src_address = 0;
	feedback_message.protocol = 0;
	feedback_message.result = IP_FOUND;
	
	char send_buf[1024];
	int len = 0;
	len = feedback_message.generate_stream(send_buf, 1024);
	int send_ret = send(la_sock, send_buf, len, 0);
	//route_discovery_done(rrepOb->getDestIP(), ASL_ROUTE_FOUND);

	/* also remove the rreq timer */
	int ret = timer_Q.removeRREQTimer((rrepOb->getDestIP()));
	if(ret == LR_TIMER)
	{
		/* remove the dest from local repair list ..
		 * also see if a route err need to be generated */

		local_repair.deleteFromList(rrepOb->getDestIP());

		rtable_entry	*rtEntry;
		rtEntry = findRoute(rrepOb->getDestIP());

		if((rtEntry != NULL) && (rtEntry->getHopCnt() > rtEntry->getLastHopCnt()))
			aodvOb.generateRERR(rrepOb->getDestIP(),1, false);
	}
	return;
}

rtable_entry	*routingTable::findActiveRoute(u_int32_t dest)
{
	rtable_entry	*rtEntry;
	u_int64_t		currtime = getcurrtime();

#ifdef DEBUG
	cout << "routingTable: findActiveRoute: entered dets is " << getDotIP(dest) << endl;
#endif

	rtEntry = findRoute(dest);
	if(rtEntry != NULL)
	{
		if((rtEntry->getLifeTime() > currtime) &&
			(rtEntry->getRFlags() == ACTIVE_ROUTE))
		{
#ifdef DEBUG
			cout << "routingTable: findActiveRoute: got a route" << endl;
#endif
			return rtEntry;
		}
		else
		{
#ifdef DEBUG
			cout << "routingTable: findActiveRoute: did not get a route" << endl;
#endif
			return NULL;
		}
	}
	return NULL;
}

rtable_entry	*routingTable::findValidRoute(u_int32_t dest, u_int32_t seqNum)
{
	rtable_entry	*rtEntry;

	rtEntry = findRoute(dest);
	if(rtEntry != NULL)
	{	
		if((rtEntry->getDestSeqNum() >= seqNum) && 
			(rtEntry->getRFlags()== ACTIVE_ROUTE))
			return rtEntry;
		else
			return NULL;
	}
	else
		return NULL;
}

/* update/add route table entry for neighbor during reboot */
void		routingTable::rebootNeighborUpdate(u_int32_t neighbor, u_int32_t seq, u_int64_t life_time)
{

	rtable_entry	*rtEntry, rtEntryNew;

#ifdef DEBUG
	cout << "routingTable: rebootNeighborUpdate: entered " << endl;
#endif

	rtEntry = findRoute(neighbor);

	if(rtEntry != NULL)
	{
		rtEntry->setNextHop(neighbor);
		rtEntry->setLifeTime(life_time);
		rtEntry->setHopCnt(1);
		rtEntry->setDestSeqNum(seq);
		rtEntry->setRFlags(ACTIVE_ROUTE);
	}
	else
	{
#ifdef DEBUG
		cout << "routingTable: neighborUpdate: create entry " << endl;
#endif
		rtEntryNew.createEntry(neighbor, seq, 1, neighbor, life_time, ACTIVE_ROUTE);
		addRoute(rtEntryNew);
	}
	return;
}


/* update/add route table entry for neighbor*/
/* pass the neighbor ip and received seq number (if any) and the lifetime*/
void		routingTable::neighborUpdate(u_int32_t neighbor, u_int32_t src, u_int32_t seq, u_int64_t life_time )
{
	rtable_entry	*rtEntry, rtEntryNew;

#ifdef DEBUG
	cout << "routingTable: neighborUpdate: entered " << endl;
#endif

	rtEntry = findRoute(neighbor);

	if(rtEntry != NULL)
	{
		rtEntry->setNextHop(neighbor);
		rtEntry->setLifeTime(life_time);
		rtEntry->setHopCnt(1);
		rtEntry->setDestSeqNum(seq);
		if(rtEntry->getRFlags() != ACTIVE_ROUTE)
		{
			rtEntry->setRFlags(ACTIVE_ROUTE);
			map<u_int32_t,rtable_entry>::iterator	iter;

#ifdef DEBUG
			cout << "insertRoute: entered" << endl;
#endif
			iter = findRouteIter(rtEntry->getDestIP());
			add_kroute(iter);
		}
	}
	else
	{
#ifdef DEBUG
		cout << "routingTable: neighborUpdate: create entry " << endl;
#endif
		rtEntryNew.createEntry(neighbor, seq, 1, neighbor, life_time, ACTIVE_ROUTE);
		insertRoute(rtEntryNew);
	}

	//zyp
	LAMessage feedback_message;
	feedback_message.cmd = LA_ODS_REPLY;
	feedback_message.dst_address = neighbor;
	feedback_message.src_address = 0;
	feedback_message.protocol = 0;
	feedback_message.result = IP_FOUND;
	
	char send_buf[1024];
	int len = 0;
	len = feedback_message.generate_stream(send_buf, 1024);
	int send_ret = send(la_sock, send_buf, len, 0);
	//route_discovery_done(neighbor,ASL_ROUTE_FOUND);

	/* also remove the rreq timer */
	int ret = timer_Q.removeRREQTimer((neighbor));
	if(ret == LR_TIMER)
	{
		local_repair.deleteFromList(neighbor);

		/* no rerr with N flag need to be generated, as the received
		 * hop cnt(1) can not be greater than the old hop cnt to neighbor */
	}


	/* also set/reset the active route timer if a hello message is received */

	/*remove the old timer */

	bool setTimer = false;

	if(timer_Q.isActiveTimerInQueue(neighbor))
	{
		timer_Q.removeActiveRouteTimer(neighbor);
		setTimer = true;
	}
	else
	{
		if(src == 0) /* hello Message */
			setTimer = true;
	}

	/* set a new timer */
	if(setTimer) 
	{
		struct	timerData	tData;
		tData.type = ACTIVE_ROUTE_TIMER;
		tData.data = neighbor;
		timer_Q.set_timer(ALLOWED_HELLO_LOSS*HELLO_INTERVAL, neighborTimeoutHandler, (void *)&tData);
	}
	return;

}

void		neighborTimeoutHandler(void *data)
{
	rtable.neighbor_timeout_handler(data);
	return;
}

void		routingTable::neighbor_timeout_handler(void *val)
{
	/* put dest in local repair list.. mark the route entry as broken...
	 * set lifetime of the route entry to currtime+ACTIVE_ROUTE_TIMEOUT...
	 * also mark any other dest using this route as broken */

	struct	timerData	*tData;
	u_int32_t	dst;

	tData = (struct timerData *)val;

	dst = tData->data;

#ifdef DEBUG
	cout << "routingTable:: neighbor_timeout_handler: entered" << endl;
#endif

	local_repair.addToList(dst);

	map<u_int32_t,rtable_entry>::iterator	iter, iter_temp;

	iter = rTableMap.find(dst);

	/* check if this entry is not already expired by timeout..the two events
	 * might happen one after another without any specific ordering */

	if((iter != rTableMap.end()))
	{
		iter->second.setRFlags(BROKEN_ROUTE);
		//iter->second.setLifeTime(getcurrtime()+(ACTIVE_ROUTE_TIMEOUT-ALLOWED_HELLO_LOSS*HELLO_INTERVAL));
		iter->second.setLifeTime(getcurrtime()+(10*ACTIVE_ROUTE_TIMEOUT));
		if(iter->second.getHopCnt() == INFINITY)
		{
			iter->second.setHopCnt(iter->second.getLastHopCnt());
		}
		else
		{
			iter->second.setLastHopCnt(iter->second.getHopCnt());
			iter->second.setDestSeqNum(iter->second.getDestSeqNum()+1);
			del_kroute(iter);
		}
	}

	iter_temp = rTableMap.begin();
	while(iter_temp != rTableMap.end())
	{
		if( iter != iter_temp)
		{
#ifdef DEBUG
			cout << "routingTable: neighbor_timeout_handler: a diff entry found " << endl;
#endif
			if((iter->second.getDestIP() == iter_temp->second.getNextHop()) && (iter_temp->second.getRFlags() == ACTIVE_ROUTE))
			{
				local_repair.addToList(iter_temp->second.getDestIP());
				iter_temp->second.setRFlags(BROKEN_ROUTE);
				iter_temp->second.setLifeTime(getcurrtime()+ACTIVE_ROUTE_TIMEOUT);
				iter_temp->second.setLastHopCnt(iter_temp->second.getHopCnt());
				iter_temp->second.setDestSeqNum(iter_temp->second.getDestSeqNum()+1);

				del_kroute(iter_temp);
			}
		}
		iter_temp++;
	}

#ifdef DEBUG
	cout << "routingTable: neighbor_timeout_handler: exiting " <<endl;
#endif

	return;
}

void		routingTable::deleteKernelRoute(u_int32_t dst)
{
	map<u_int32_t,rtable_entry>::iterator	iter;

	iter = rTableMap.find(dst);
	if(iter != rTableMap.end())
	{
		del_kroute(iter);
		return;
	}
	return;
}


/* update/delete stale entries from the route table */
void		routingTable::refreshEntries()
{
	map<u_int32_t,rtable_entry>::iterator	iter, iter_temp, iter_temp2, iterNew;
	int			time_idle, temp_time_idle;
	u_int64_t	currtime = getcurrtime();

#ifdef DEBUG
	cout << "routingTable: refreshEntries: entered" << endl;
#endif

	iter = rTableMap.begin();
	while(iter != rTableMap.end())
	{
		iter++;
		iterNew=iter;
		iter--;
		
#ifdef DEBUG
		cout << "routingTable: refreshEntries: dest ip " << getDotIP(iter->second.getDestIP()) << endl;
#endif
		if(iter->second.getLifeTime() <= currtime)
		{
#ifdef DEBUG
			cout << "routingTable: refreshEntries: entry is about to expire " << endl;
#endif
			
			if(iter->second.getHopCnt() == INFINITY)
			{
#ifdef DEBUG
				cout << "routingTable: refreshEntries: INFINITY hopcnt " << endl;
#endif
				/* delete entry from the routing table*/
				rTableMap.erase(iter);
			}
			else
			{
				/* query route_check module for the idle time */
				/* first query the actual destination entry (with flag 1)..
				 * and then if this node is one hop away then..
				 * query all those nodes (with flag 0)for which this 
				 * node acts as next hop */

				time_idle = FO_query_route_idle_time(iter->second.getDestIP(),1);
				if(time_idle < 0)
					time_idle = 2*ACTIVE_ROUTE_TIMEOUT;

				if(iter->second.getHopCnt() == 1)
				{
#ifdef DEBUG
					cout << "routingTable: refreshEntries: hop cnt is 1, time_idle " << time_idle << endl;
#endif
					iter_temp2 = rTableMap.begin();
					while(iter_temp2 != rTableMap.end())
					{
						if( iter != iter_temp2)
						{
							if( iter->second.getDestIP() == iter_temp2->second.getNextHop())
							{
								temp_time_idle = FO_query_route_idle_time(iter_temp2->second.getDestIP(),0);
								if((temp_time_idle != -1) && (temp_time_idle < time_idle))
									time_idle = temp_time_idle;
							}
						}
						iter_temp2++;
					}
				}
					
#ifdef DEBUG
				cout << "routingTable: refreshEntries: non INFINITY hopcnt, time_idle " << time_idle << endl;
#endif
				if(time_idle < ACTIVE_ROUTE_TIMEOUT)
				{
#ifdef DEBUG
					cout << "routingTable: refreshEntries: update lifetime "  << endl;
#endif
					rtable_entry	*rtEntry;
					/* also update the lifetime for next hop and the prev hop */
					iter->second.setLifeTime((currtime-time_idle)+ACTIVE_ROUTE_TIMEOUT);

					/* here we don't need to check is the route is an ACTIVE_ROUTE
					 * or a BROKEN_ROUTE because route can not be a broken route if
					 * the time_idle is smaller than ACTIVE_ROUTE_TIMEOUT */

					rtEntry = findRoute(iter->second.getNextHop());
					rtEntry->setLifeTime((currtime-time_idle)+ACTIVE_ROUTE_TIMEOUT);

					/* we don't need to update prev hop lifetime..
					 * whenever the prev hop entry expires..
					 * query_route_idle_time() is called either to update
					 * or delete the entry */
				}
				else
				{
					/* make the route entry invalid */
					/* this would also involve broken routes */
					u_int8_t	prevHopCnt;
					RERR		rerrOb;

#ifdef DEBUG
					cout << "routingTable: refreshEntries: invalidate entry " <<  endl;
					cout << "routingTable: refreshEntries: delete kernel route " <<  endl;
					/* also delete the kernel rtable entry */
					
					cout << "routingTable: refreshEntries: host ip is " << getDotIP(iter->second.getDestIP()) << endl; 
#endif

					prevHopCnt = iter->second.getHopCnt();

					if(iter->second.getRFlags() == ACTIVE_ROUTE)
					{
						del_kroute(iter);
						iter->second.setDestSeqNum((iter->second.getDestSeqNum()+1));
						iter->second.setLastHopCnt(iter->second.getHopCnt());
					}


#ifdef DEBUG
					cout << "routingTable: refreshEntries: dest seq num is " << iter->second.getDestSeqNum() << endl;
#endif
					iter->second.setHopCnt(INFINITY);
					iter->second.setLifeTime(currtime+DELETE_PERIOD);
					iter->second.setRFlags(INVALID_ROUTE);

					/* also remove the entry from the local repair list */
					if((iter->second.getRFlags() == BROKEN_ROUTE))
					{
						local_repair.deleteFromList(iter->second.getDestIP());
					}

					if(prevHopCnt == 1)
					{
						/* this node was a neighbor before , update entries for
						 * other destinations using this neighbor as next hop */
#ifdef DEBUG
						cout << "routingTable: refreshEntries: neighbor node " << endl;
#endif
						iter_temp = rTableMap.begin();
						while(iter_temp != rTableMap.end())
						{
							if( iter != iter_temp)
							{
#ifdef DEBUG
								cout << "routingTable: refreshEntries: a diff entry found " << endl;
#endif
								if((iter->second.getDestIP() == iter_temp->second.getNextHop()) && (iter_temp->second.getHopCnt() != INFINITY))
								{

									/* also delete the kernel rtable entry */
									if(iter_temp->second.getRFlags() == ACTIVE_ROUTE)
									{
										del_kroute(iter_temp);
										iter_temp->second.setLastHopCnt(iter_temp->second.getHopCnt());
										iter_temp->second.setDestSeqNum((iter_temp->second.getDestSeqNum()+1));
									}
#ifdef DEBUG
									cout << "routingTable: refreshEntries: modify entry for " <<getDotIP(iter_temp->second.getDestIP()) << endl;
#endif
									/* update this route table entry */
									iter_temp->second.setHopCnt(INFINITY);
									iter_temp->second.setLifeTime(currtime+DELETE_PERIOD);
									iter_temp->second.setRFlags(INVALID_ROUTE);

									/* also remove the entry from the local repair list */
									if((iter_temp->second.getRFlags() == BROKEN_ROUTE))
									{
										local_repair.deleteFromList(iter_temp->second.getDestIP());
									}

								}
							}
							iter_temp++;
						}
					}
				}
			}
		}
		iter = iterNew;
	}
	return;
}


int		routingTable::del_kroute( map<u_int32_t,rtable_entry>::iterator iter )
{
	  struct rtentry  *new_krtentry;

#ifdef DEBUG
	  cout << "del_kroute: Entered" << endl;
#endif
	  if ((new_krtentry = gen_krtentry(iter)) == NULL)
	  {
		  cout << "del_kroute: error in gen_krtentry " << endl;
	    /* del_kroute failed */
	    return -1;
	  }

	  if(ioctl(ksock, SIOCDELRT, (char*) new_krtentry) == -1)
	  {
	    /* SIOCDELRT failed */
#ifdef DEBUG
		  cout << "routingTable: del_kroute: ioctl failed " << endl;
        perror("Error in ioctl- deleting routes");
#endif
	    return -1;
	  }

	  return 0;
	  free(new_krtentry);

}

int		routingTable::add_kroute( map<u_int32_t,rtable_entry>::iterator iter )
{
	int ifindex = if_nametoindex(interface);
	FO_add_a_route(iter->second.getDestIP(), 32, iter->second.getNextHop(), ifindex, 100, 0);
// 	/*
// 	route_add(iter->second.getDestIP(), iter->second.getNextHop(), interface);
// 	return 0;
// 	*/
// 	  struct rtentry  *new_krtentry;

// #ifdef DEBUG
// 	  cout << "add_kroute: Entered" << endl;
// #endif

// 	  if ((new_krtentry = gen_krtentry(iter)) == NULL)
// 	  {
// 	     cout << "Error in gen_krtentry" << endl;
// 	    return -1;
// 	  }

// 	  if(ioctl(ksock, SIOCADDRT,(char*) new_krtentry) == -1)
// 	  {
// #ifdef DEBUG
// 		  cout << "routingTable: add_kroute: ioctl failed " << endl;
//         perror("Error in ioctl- adding routes");
// #endif
// 	    return -1;
// 	  }

// 	  free(new_krtentry);
	  return 0;
}

struct rtentry *routingTable::gen_krtentry( map<u_int32_t,rtable_entry>::iterator iter )
{

	  struct rtentry *new_rtentry;
	  struct sockaddr_in dst;
	  struct sockaddr_in gw;
	  struct sockaddr_in genmask;
	  unsigned int netmask=0xffffffff;

#ifdef DEBUG
	  cout << "gen_krtentry: Entered" << endl;
#endif

	  if((new_rtentry = (struct rtentry *)malloc(sizeof(struct rtentry))) == NULL)
	  {
		perror("Error in malloc");
	   	return NULL; 
	  }

	  bzero((void *)new_rtentry,sizeof(struct rtentry));
	  bzero((void *)&dst,sizeof(struct sockaddr_in));
	  bzero((void *)&gw,sizeof(struct sockaddr_in));
	  bzero((void *)&genmask,sizeof(struct sockaddr_in));

	  dst.sin_family = AF_INET;
	  gw.sin_family = AF_INET;
	  genmask.sin_family = AF_INET;

	  dst.sin_addr.s_addr = iter->second.getDestIP();

	  struct in_addr 	addr_temp;
	  addr_temp.s_addr = g_my_ip;
#ifdef DEBUG
	  cout << "gen_krtentry: Local Host is   " << inet_ntoa(addr_temp) << endl;
	  addr_temp.s_addr = iter->second.getDestIP();
	  cout << "gen_krtentry: DstIP is   " << inet_ntoa(addr_temp) << endl;
	  addr_temp.s_addr = iter->second.getNextHop();
	  cout << "gen_krtentry: Nxt Hop is   " << inet_ntoa(addr_temp) << endl;
#endif

	  if( iter->second.getHopCnt() == 1)
	  {
#ifdef DEBUG
		cout << " routingTable: gen_krtentry: 1 hop cnt " << endl;
#endif
	  	gw.sin_addr.s_addr = g_my_ip;
	  	//gw.sin_addr.s_addr = iter->second.getNextHop();
	  }
	  else
	  {
#ifdef DEBUG
		cout << " routingTable: gen_krtentry: multi hop cnt " << endl;
#endif
	  	gw.sin_addr.s_addr = iter->second.getNextHop();
	  }

	  genmask.sin_addr.s_addr = netmask;
	  //genmask.sin_addr.s_addr = inet_addr("255.255.255.255");

	  new_rtentry->rt_flags = RTF_UP | RTF_HOST | RTF_GATEWAY;
	  new_rtentry->rt_metric = (short)iter->second.getHopCnt();
	  new_rtentry->rt_dev = interface;
	  new_rtentry->rt_dst = *(struct sockaddr*) &dst;
	  new_rtentry->rt_gateway = *(struct sockaddr*) &gw;
	  new_rtentry->rt_genmask = *(struct sockaddr*) &genmask;

	  return new_rtentry;
}

