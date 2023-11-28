/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

int	RERR::getN()
{
	if(N==1)
		return 1;
	else
		return 0;
}

u_int32_t	RERR::getUnrchDest()
{
	list<unrchDest>::iterator	iter;

	iter=unreachable_dest.begin();
	if((dest_cnt) == 1)
	{
		return iter->getDestIP();
	}
	return 0;
}


u_int32_t	RERR::getUnrchDestSeqNum()
{
	list<unrchDest>::iterator	iter;

	iter=unreachable_dest.begin();
	if((dest_cnt) == 1)
	{
		return iter->getDestSeqNum();
	}
	return 0;
}


void	RERR::setUnrchDest(u_int32_t dst, u_int32_t seq)
{
	unrchDest		uDest;
	
#ifdef DEBUG
	cout << "RERR: setUnrchDest: entered" << endl;
#endif

	uDest.setDestIP(dst);
	uDest.setDestSeqNum(seq);

	unreachable_dest.push_back(uDest);
	return;
}

void	RERR::copyIntoBuf(char	*buf)
{
	int i;
	u_int8_t	temp_N;
	u_int32_t	dst, dstSeqNum;

	buf[0]=type;
	temp_N = getN();
	temp_N = temp_N << 7;
	buf[1] = temp_N;
	buf[2] = reserved2;
	buf[3] = dest_cnt;

	list<unrchDest>::iterator	iter;
	iter = unreachable_dest.begin();

#ifdef DEBUG
	cout << "RERR: copyIntoBuf: entered" << endl;
	cout << "RERR: copyIntoBuf: type is" << (int)type << endl;
#endif

	i=0;
	while(iter != unreachable_dest.end())
	{
#ifdef DEBUG
		cout << "RERR: copyIntoBuf: inside while" << endl;
#endif

		dst = (iter->getDestIP());
		dstSeqNum = (iter->getDestSeqNum());

#ifdef DEBUG
		cout << "RERR: copyIntoBuf: dst is" << getDotIP(dst) << endl;
		cout << "RERR: copyIntoBuf: dst seq num is" << dstSeqNum << endl;
#endif

		memcpy((buf+4+i*8),(void *)&dst,4);
		memcpy((buf+8+i*8),(void *)&dstSeqNum,4);
		i+=1;
		iter++;
	}
	return;	
}

/* update route table entries for the invalid entries(with INFINITY metric count)
 * with the sequence number information received in the rerr message */
void	RERR::updateInvalidEntries()
{
	list<unrchDest>::iterator 	iter;
	u_int32_t	seqNum;

#ifdef DEBUG
	cout << "RERR: updateInvalidEntries: entered " << endl;
#endif

	iter = unreachable_dest.begin();

	while( iter != unreachable_dest.end() )
	{
		rtable_entry		*rtEntry;

		rtEntry = rtable.findRoute(iter->getDestIP());

		/* update the seq num of invalid routes to the max of existing and received*/

		if(rtEntry != NULL && (rtEntry->getHopCnt() == INFINITY))
		{
			seqNum = MAX(iter->getDestSeqNum(),rtEntry->getDestSeqNum());
			rtEntry->setDestSeqNum(seqNum);
		}
		iter++;
	}
	return;

}

/* update route table entries for the valid entries
 * with the sequence number information received in the rerr message */
void 	RERR::updateValidEntries(u_int32_t prevHop)
{
	list<unrchDest>::iterator 	iter;

	u_int64_t	currtime = getcurrtime();

#ifdef DEBUG
	cout << "RERR: updateValidEntries: entered " << endl;
#endif
	iter = unreachable_dest.begin();

	while( iter != unreachable_dest.end() )
	{
		rtable_entry		*rtEntry;

		rtEntry = rtable.findRoute(iter->getDestIP());

		/* update the seq num of valid routes */

		/* if a RERR is received for a broken route, it should get
		 * invalidated */
		if(rtEntry != NULL && (rtEntry->getHopCnt() != INFINITY))
		{
			if(rtEntry->getNextHop() == prevHop)
			{
				/* delete the kernel route as well */
				if(rtEntry->getRFlags() == ACTIVE_ROUTE)
					rtable.deleteKernelRoute(rtEntry->getDestIP());
				else if(rtEntry->getRFlags() == BROKEN_ROUTE)
				{
					/* remove entry from the local repair list */
					local_repair.deleteFromList(rtEntry->getDestIP());
				}

				/* update the route table entry */
				rtEntry->setLastHopCnt(rtEntry->getHopCnt());
				rtEntry->setHopCnt(INFINITY);
				rtEntry->setDestSeqNum(iter->getDestSeqNum());
				rtEntry->setLifeTime(currtime+DELETE_PERIOD);
				rtEntry->setRFlags(INVALID_ROUTE);
			}
		}
		iter++;
	}
	return;

}

/* create a RERR message */
void	RERR::createRERR(char	*buf)
{
	u_int32_t	dst, seq;
	u_int8_t	i=0, temp_N;
	
#ifdef DEBUG
	cout << "RERR: createRERR: entered " << endl;
#endif

	type = buf[0];
	temp_N = buf[1];
	temp_N = temp_N >> 7;
	setNFlag(temp_N);
	reserved2 = buf[2];
	dest_cnt = buf[3];

#ifdef DEBUG
	cout << "RERR: createRERR: dest cnt is" << (int)dest_cnt << endl;
#endif
	i=0;
	while(i < dest_cnt)
	{
		memcpy(&dst,(buf+4+i*8),4);
		memcpy(&seq,(buf+8+i*8),4);
		setUnrchDest(dst,seq);
		i+=1;
	}
}

/* process received RERR message and create a new RERR message if needed
 * prevHop is the node from which the rerr message is received */
bool	RERR::createNewRERR(RERR *rerrOb, u_int32_t prevHop,  u_int32_t *addr)
{
	u_int32_t	dst;
	u_int8_t	cnt=0;
	bool	transmit = false, brdFlag=false;
	u_int32_t	precCnt=0, tempPrecCnt;
	u_int32_t	*tempAddr;
	u_int64_t	currtime;

	addr = NULL;

#ifdef DEBUG
	cout << "RERR: createNewRERR: entered " << endl;
#endif

	type = rerrOb->type;
	N = rerrOb->N;
	reserved1 = rerrOb->reserved1;
	reserved2 = rerrOb->reserved2;

	currtime = getcurrtime();

	tempAddr = (u_int32_t *)malloc(sizeof(u_int32_t));

	list<unrchDest>::iterator	iter;
	iter = rerrOb->unreachable_dest.begin();
	
	/* update route table entries with the information in the unreachable 
	 * destination list */
	while(iter != rerrOb->unreachable_dest.end())
	{
		dst = iter->getDestIP();
		rtable_entry	*rtEntry;

		rtEntry = rtable.findRoute(dst);
		if(rtEntry != NULL && (rtEntry->getHopCnt() != INFINITY))
		{
			/* look for an active route to that destination */
			if(rtEntry->getNextHop() == prevHop)
			{
				/* delete the kernel route as well */
				if(rtEntry->getRFlags() == ACTIVE_ROUTE)
					rtable.deleteKernelRoute(rtEntry->getDestIP());
				else if(rtEntry->getRFlags() == BROKEN_ROUTE)
				{
					/* remove entry from the local repair list */
					local_repair.deleteFromList(dst);
				}

				/* update the route table entry */
				rtEntry->setLastHopCnt(rtEntry->getHopCnt());
				rtEntry->setHopCnt(INFINITY);
				rtEntry->setDestSeqNum(iter->getDestSeqNum());
				rtEntry->setLifeTime(currtime+DELETE_PERIOD);
				rtEntry->setRFlags(INVALID_ROUTE);

				cnt++;

				/* add this to the new rerr */
				setUnrchDest(rtEntry->getDestIP(), rtEntry->getDestSeqNum());

				tempPrecCnt = rtEntry->getPrecCnt();
				if(tempPrecCnt > 1)
				{
					brdFlag = true;
					addr=NULL;
					precCnt+=tempPrecCnt;
				}

				if(!brdFlag)
				{
					if(tempPrecCnt == 1)
					{
						if(addr != NULL)
						{
							rtEntry->getPrecAddr(tempAddr);
							if(*tempAddr != *addr)
							{
								precCnt+=tempPrecCnt;
								addr = NULL;
								brdFlag = true;
							}
						}
						else
						{
							rtEntry->getPrecAddr(addr);
							precCnt+=tempPrecCnt;
						}
					}
				}
			}
		}
		iter++;
	}

	free(tempAddr);
	setDestCnt(cnt);

	if(precCnt > 0)
		transmit = true;

	/* transmit the new rerr if at least one precursor exist */
	return transmit;
}
