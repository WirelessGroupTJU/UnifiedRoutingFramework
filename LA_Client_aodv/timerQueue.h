/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef TIMERQUEUE_H
#define TIMERQUEUE_H

#include "timer.h"
using namespace std;
typedef void (*timer_hfunc_t) (void *);


class timerQueue{

	list<timer>		timerQ;

	public:

	list<timer>		getTimerQ() { return timerQ;}
	int timer_init();
	int timer_add(u_int64_t timeVal, timer_hfunc_t func, void *dataVal);
	int timer_delete();
	int set_timer(u_int64_t timeout,timer_hfunc_t fun, void *data);
	int set_timer_first(u_int64_t timeout,timer_hfunc_t fun, void *data);

	int		scheduleTimer();
	int		setRealTimer(u_int64_t timeout);
	void	resetRebootTimer();
	bool	isSetNow(list<timer>::iterator iterArg);
	int		removeRREQTimer(u_int32_t dst);
	void	removeRREP_ACK_Timer(u_int32_t dst);
	void	removeActiveRouteTimer(u_int32_t dst);
	bool	isTimerSetNow(list<timer>::iterator iterArg);
	void	setLowestTimer(u_int64_t timeVal);
	bool	isActiveTimerInQueue(u_int32_t dst);

};

#endif
