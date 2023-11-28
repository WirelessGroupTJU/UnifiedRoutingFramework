#ifndef ND_TIME_RQUEUE_H
#define ND_TIME_RQUEUE_H

#include <list>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include "nd_interface.h"
#include "nd_timer.h"

using namespace std;


class timerQueue{

	list<timer>		timerQ;

	public:

	list<timer>		getTimerQ() { return timerQ;}

	int 	timer_init();

	int 	timer_add(u_int64_t currtime, u_int64_t timeVal, timer_hfunc_t func, void *dataVal);
	void 	setLowestTimer(u_int64_t currtime, u_int64_t timeVal);
	int		setRealTimer(u_int64_t timeout);
	int 	set_timer(u_int64_t timeout,timer_hfunc_t fun, void *data);

	bool	is_set_now(list<timer>::iterator iterArg);
	void	remove_neighbor_entry_timer(uint32_t dst_address, unsigned char* interface_name);

	int		scheduleTimer();

	void	clean_timer_list();

	uint64_t getcurrtime();
};

#endif
