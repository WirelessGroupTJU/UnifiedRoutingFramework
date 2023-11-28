#include "nd_timer_queue.h"

int timerQueue::timer_init()
{
	return 0;
}

/* 
	add an entry to the timer queue 
	只负责添加timer到队列中，不负责linux系统定时器是否需要覆盖
*/
int timerQueue::timer_add(u_int64_t currtime, u_int64_t timeVal, timer_hfunc_t func, void *dataVal)
{
	timer	timerOb;
	struct	timerData	*tData, *tempData;

	tempData = (struct timerData *)dataVal;

	tData = (struct timerData *)malloc(sizeof(struct timerData));
	//tData = (struct timerData *)dataVal;
	tData->type = tempData->type;
	tData->data = tempData->data;
	tData->contain = tempData->contain;
	
	// tData->entry = tempData->entry;

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

// 在添加timer进入队列后，需要判断当前队列头是不是新加入的那个timer，如果是那么就需要重新设置linux系统定时器
void	timerQueue::setLowestTimer(u_int64_t currtime, u_int64_t timeVal)
{
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

	//设置真正的linux定时器，setitimer在超时之前再次被调用将会覆盖上一次的定时间隔
	if (setitimer(ITIMER_REAL, &itval, (struct itimerval *)NULL) < 0) {
			cout << "ERROR: setting periodic timer"  << endl;
			return -1;
	}

	return 0;
}

//定时器添加的一个完整过程
int timerQueue::set_timer(u_int64_t timeVal, timer_hfunc_t fun, void *dataVal )
{
#ifdef DEBUG
	cout << "timerQueue: set_timer: entered" << endl;
#endif
	u_int64_t currtime = getcurrtime();

	/* add to the timer list */
	timer_add(currtime, timeVal, fun, dataVal);

	/* find the timer with minimum timeout and set the timer for that value */
	setLowestTimer(currtime, timeVal);

	return 0;
}

bool	timerQueue::is_set_now(list<timer>::iterator iterArg){
	struct timerData	*data1, *data2;

	list<timer>::iterator	iter;
	iter=timerQ.begin();

	data1 = (struct timerData*)iter->data;
	data2 = (struct timerData*)iterArg->data;

	if(data1->type == data2->type && data1->type == data2->type && data1->contain == data2->contain)
		return true;
	else 
		return false;
}

/*delete a one-hop neighbor-entry when a timer is timeout*/
void timerQueue::remove_neighbor_entry_timer(uint32_t dst_address, unsigned char* interface_name){
	struct timerData	*tData;

	#ifdef DEBUG
		cout << "timerQueue: removeActiveRouteTimer: entered dst is" << getDotIP(dst) << endl;
	#endif
	list<timer>::iterator	iter, newIter;
	iter=timerQ.begin();

	//find a one-hop entry timer
	while(iter != timerQ.end()){
		iter++;
		newIter = iter;
		iter--;

		tData = (struct timerData*)iter->data;
		InterfaceEntry* now_entry = (InterfaceEntry*)tData->contain;
		unsigned char* now_entry_name = now_entry->name;

		if((tData->type == TIMER_TYPE_ENTRY) && (tData->data==dst_address) && !strcmp((const char*)now_entry_name, (const char*)interface_name)){
	#ifdef DEBUG
				cout << "timerQueue: removeActiveRouteTimer: found a timer entry" << endl;
	#endif
			//if this timer is the first timer, we need to reset real timer
			if(is_set_now(iter)){
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

	if((iter->timeout > 0) && (iter->timeout > currtime) && (iter != timerQ.end()))	//还没到点的启动定时器
		setRealTimer((iter->timeout-currtime));

	return 0;
	
}

void timerQueue::clean_timer_list(){
	timerQ.clear();
}

/* Return current time in msecs */
uint64_t timerQueue::getcurrtime(){
	struct timezone tz;
	struct timeval tv;
	
	if (gettimeofday(&tv, &tz) < 0)
	/* Couldn't get time of day */
	return 0;

	//return ((u_int32_t)tv.tv_sec) * 1000 + ((u_int32_t)tv.tv_usec) / 1000;
	return ((u_int64_t)tv.tv_sec) * 1000 + ((u_int64_t)tv.tv_usec) / 1000;
}

