/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"
#include "timer.h"

int timerQueue::timer_init()
{
	return 0;
}

/* add an entry to the timer queue */ //添加到list中
int timerQueue::timer_add(u_int64_t timeVal, timer_hfunc_t func, void *dataVal)
{
	timer	timerOb;
	struct	timerData	*tData, *tempData;
	u_int64_t	currtime=getcurrtime();

#ifdef DEBUG
	cout << "timer_add: entered" << endl;
#endif

	tempData = (struct timerData *)dataVal;

	tData = (struct timerData *)malloc(sizeof(struct timerData));
	//tData = (struct timerData *)dataVal;
	tData->type = tempData->type;
	tData->data = tempData->data;

#ifdef DEBUG
	cout << "timer_add: timer type is " << tData->type << "timeout is  " << timeVal << "currtime is " << currtime <<endl; 
#endif
	timerOb.timeout = currtime + timeVal; /* absolute time in msec */
#ifdef DEBUG
	cout << "timer_add: absolute timeout is  " << timerOb.timeout <<endl; 
#endif
	timerOb.handler = func;
	timerOb.data = tData;

	/* insert at the right place 按顺序插入到list中，list中timeout是从小到大，也就是说：先到点的在前头*/		
	list<timer>::iterator	iter;
	if(timerQ.size() == 0)
	{
#ifdef DEBUG
		cout << "timer_add: timer size 0 " << endl;
#endif
		timerQ.push_back(timerOb);
	}
	else
	{
#ifdef DEBUG
		cout << "timer_add: timer size " << timerQ.size() << endl;
#endif

		iter=timerQ.begin();
		while(timerOb.timeout > iter->timeout && (iter != timerQ.end()))	// 找到合适的位置
		{
			iter++;
#ifdef DEBUG
			cout << "timerQueue: timer_add: within while " << endl;
#endif
		}
		if(iter == timerQ.end())	// 如果到末尾了，就插入到list最末尾
		{
#ifdef DEBUG
			cout << "timerQueue: timer_add:  iter reached end " << endl;
#endif
			timerQ.push_back(timerOb);
		}
		else		//如果不是末尾，则插入到当前这个iter位置。
		{
#ifdef DEBUG
			cout << "timerQueue: timer_add:  insert in the middle" << endl;
#endif
			timerQ.insert(iter,timerOb);	//插入到iter这个迭代器所处的位置，原来这个位置的向后移动。
		}
	}
	return 0;	
}

/* handler function which is called when the global timer expires */
int timerQueue::scheduleTimer()	//处理list列表，小于当前时间（到点了）的iter->handler，大于当前时间（没到点）的启动定时器
{
	list<timer>::iterator	iter,tmp_iter;
	iter=timerQ.begin();

	u_int64_t	currtime = getcurrtime();

#ifdef DEBUG
	cout << "scheduleTimer: entered timer size is " << timerQ.size() << " timeout is " << iter->timeout  << "currtime is" << currtime << endl;
#endif

	while((iter->timeout <= currtime) && (iter != timerQ.end()))	// 已经到点（过点）了，处理一下。
	{

		iter->handler(iter->data);

		iter++;
		tmp_iter = iter;
		iter--;

		timerQ.erase(iter);
		iter = tmp_iter;
	}

	if((iter->timeout > 0) && (iter->timeout > currtime) && (iter != timerQ.end()))	//还没到点的启动
		setRealTimer((iter->timeout-currtime));

	return 0;
	
}


int timerQueue::set_timer(u_int64_t timeVal, timer_hfunc_t fun, void *dataVal )
{
#ifdef DEBUG
	cout << "timerQueue: set_timer: entered" << endl;
#endif

	/* add to the timer list */
	timer_add(timeVal, fun, dataVal);

	/* find the timer with minimum timeout and set the timer for that value */
	setLowestTimer(timeVal);

	return 0;
}
//第一次设置时间
int timerQueue::set_timer_first(u_int64_t timeVal, timer_hfunc_t fun, void *dataVal )
{

#ifdef DEBUG
	cout << "set_timer_first: entered" << endl;
#endif
	/* add to the timer list */
	timer_add(timeVal, fun, dataVal);

	/* find the timer with minimum timeout and set the timer for that value */
	setRealTimer(timeVal);	//真正启动linux定时器
	return 0;
}
// list<timer>是一个队列，timer都在这里排队，当头一个（lowest）到点之后，启动真正linux定时器
void	timerQueue::setLowestTimer(u_int64_t timeVal)
{
	u_int64_t	currtime = getcurrtime();
	struct timerData	*tData;
	

#ifdef DEBUG
	cout << "timerQueue: setLowestTimer: entered " << endl;
#endif
	list<timer>::iterator	iter;
	iter=timerQ.begin();

	tData = (struct timerData*) iter->data;
#ifdef DEBUG
	cout << "setLowestTimer: timer queue size is " << timerQ.size() << endl;
	cout << "setLowestTimer: timer type is " << tData->type << endl;
	cout << "setLowestTimer: timeVal is " << timeVal << endl;
	cout << "setLowestTimer: first entry timeout is " << iter->timeout << endl;
	cout << "setLowestTimer: current timeout is " << (currtime+timeVal) << endl;
#endif

	if(iter->timeout == (currtime+timeVal))
	{
		/* reset the timer to the timeval timeout */
#ifdef DEBUG
		cout << "timerQueue: setLowestTimer: resetting timer " << endl;
#endif
		setRealTimer(timeVal);
	}
	return;
}

/* timeout is the absolute time in ms */
int	timerQueue::setRealTimer(u_int64_t timeout)	//设置真正的linux定时器，
{
	struct itimerval itval;
	long	time_sec, time_usec;

#ifdef DEBUG
	cout << " setRealTimer: entered timeout is " << timeout <<  endl;
#endif
	time_sec = timeout/1000;
	time_usec = (timeout - time_sec*1000)*1000;
	//itval.it_interval.tv_sec = time_sec;
	itval.it_interval.tv_sec = 0;
	itval.it_value.tv_sec = time_sec;
	//itval.it_interval.tv_usec = time_usec;
	itval.it_interval.tv_usec = 0;
	itval.it_value.tv_usec = time_usec;

	//设置真正的linux定时器
	if (setitimer(ITIMER_REAL, &itval, (struct itimerval *)NULL) < 0) {
			cout << "ERROR: setting periodic timer"  << endl;
			return -1;
	}

	return 0;
}

void	timerQueue::resetRebootTimer()	//-OK
{
	struct timerData	*data;

#ifdef DEBUG
	cout << "timerQueue: resetRebootTimer: entered"  << endl;
#endif

	list<timer>::iterator	iter;
	iter=timerQ.begin();

	while(iter != timerQ.end())
	{
		data = (struct timerData*)iter->data;
		if(data->type == REBOOT_TIMER)
		{
			iter->timeout = getcurrtime()+DELETE_PERIOD;
			if(isSetNow(iter))
				setRealTimer(iter->timeout);
			return;
		}
		iter++;
	}
	return;
}

bool	timerQueue::isSetNow(list<timer>::iterator iterArg)
{

	struct timerData	*data1, *data2;

	list<timer>::iterator	iter;
	iter=timerQ.begin();

	data1 = (struct timerData*)iter->data;
	data2 = (struct timerData*)iterArg->data;

	if(data1->type == data2->type)
		return true;
	else 
		return false;
}

int	timerQueue::removeRREQTimer(u_int32_t dst)
{
	struct timerData	*tData;
	int		ret;

#ifdef DEBUG
	cout << "timerQueue: removeRREQTimer: entered dst is" << getDotIP(dst) << endl;
#endif
	list<timer>::iterator	iter, newIter;
	iter=timerQ.begin();

	while(iter != timerQ.end())
	{
		iter++;
		newIter = iter;
		iter--;

		tData = (struct timerData*)iter->data;

#ifdef DEBUG
		cout << "timerQueue: removeRREQTimer: inside while timer type is" << tData->type << endl;
		cout << "timerQueue: removeRREQTimer: inside while dst is" << getDotIP(tData->data) << endl;
#endif
		if((tData->type == RREQ_TIMER || tData->type == LR_TIMER) && (tData->data==dst))
		{
#ifdef DEBUG
			cout << "timerQueue: removeRREQTimer: found a timer entry" << endl;
#endif
			if(isTimerSetNow(iter))
			{
#ifdef DEBUG
				cout << "timerQueue: removeRREQTimer: rreq timer is set now " <<endl;
#endif
				if(newIter != timerQ.end())
					setRealTimer((newIter->timeout - getcurrtime()));
			}
			if(tData->type == LR_TIMER)
				ret = LR_TIMER;
			else
				ret = RREQ_TIMER;

			free(iter->data);
			timerQ.erase(iter);
			return ret;
		}
		iter = newIter;
	}
	return 0;
}

bool	timerQueue::isTimerSetNow(list<timer>::iterator iterArg)
{
	struct timerData	*data1, *data2;

	list<timer>::iterator	iter;
	iter=timerQ.begin();

	data1 = (struct timerData*)iter->data;
	data2 = (struct timerData*)iterArg->data;

	if((data1->type == data2->type) && (data1->data == data2->data))
		return true;
	else 
		return false;
}

void	timerQueue::removeRREP_ACK_Timer(u_int32_t dst)
{
	struct timerData	*tData;

#ifdef DEBUG
	cout << "timerQueue: removeRREP_ACK_Timer: entered" << endl;
#endif
	list<timer>::iterator	iter, newIter;
	iter=timerQ.begin();

	while(iter != timerQ.end())
	{
		iter++;
		newIter = iter;
		iter--;

		tData = (struct timerData*)iter->data;
		if((tData->type == RREP_ACK_TIMER ) && (tData->data==dst))
		{
#ifdef DEBUG
			cout << "timerQueue: removeRREP_ACK_Timer: found a timer entry" << endl;
#endif
			if(isTimerSetNow(iter))
			{
#ifdef DEBUG
				cout << "timerQueue: removeRREP_ACK_Timer: rrep ack timer is set now " <<endl;
#endif
				if(newIter != timerQ.end())
					setRealTimer((newIter->timeout - getcurrtime()));
			}

			free(iter->data);
			timerQ.erase(iter);
			return;
		}
		iter = newIter;
	}
	return ;
}

void	timerQueue::removeActiveRouteTimer(u_int32_t dst)
{
	struct timerData	*tData;

#ifdef DEBUG
	cout << "timerQueue: removeActiveRouteTimer: entered dst is" << getDotIP(dst) << endl;
#endif
	list<timer>::iterator	iter, newIter;
	iter=timerQ.begin();

	while(iter != timerQ.end())
	{
		iter++;
		newIter = iter;
		iter--;

		tData = (struct timerData*)iter->data;

#ifdef DEBUG
		cout << "timerQueue: removeActiveRouteTimer: inside while timer type is" << tData->type << endl;
		cout << "timerQueue: removeActiveRouteTimer: inside while dst is" << getDotIP(tData->data) << endl;
#endif

		if((tData->type == ACTIVE_ROUTE_TIMER ) && (tData->data==dst))
		{
#ifdef DEBUG
			cout << "timerQueue: removeActiveRouteTimer: found a timer entry" << endl;
#endif
			free(iter->data);
			timerQ.erase(iter);
			return ;
		}
		iter = newIter;
	}
	return ;
}


bool	timerQueue::isActiveTimerInQueue(u_int32_t dst)
{
	list<timer>::iterator	iter;
	iter=timerQ.begin();

	struct timerData	*tData;

	while(iter != timerQ.end())
	{
		tData = (struct timerData*)iter->data;

#ifdef DEBUG
		cout << "timerQueue: isActiveTimerInQueue: inside while timer type is" << tData->type << endl;
#endif

		if((tData->type == ACTIVE_ROUTE_TIMER ) && (tData->data==dst))
		{
#ifdef DEBUG
			cout << "timerQueue: removeActiveRouteTimer: found a timer entry" << endl;
#endif
			return true;
		}
		iter++;
	}
	return false;
}

