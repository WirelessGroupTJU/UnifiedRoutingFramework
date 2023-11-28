/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef TIMER_H
#define TIMER_H

typedef void (*timer_hfunc_t) (void *);


class timer{

	u_int64_t		timeout;
	//struct timeval	timeout;
	timer_hfunc_t		handler;
	void			*data;

	public:

	timer() {}

	timer(u_int64_t timer_val, timer_hfunc_t handler_ptr, void *func_data)
	{
		timeout = timer_val;
		handler = handler_ptr;
		data = func_data;
	}

	void *	getData() { return data; }
	
	friend bool operator< (const timer &t1, const timer &t2);
	friend bool operator<= (const timer &t1, const timer &t2);
	friend bool operator> (const timer &t1, const timer &t2);
	friend bool operator>= (const timer &t1, const timer &t2);
 	friend bool operator== (const timer &t1, const timer &t2);
	friend bool operator!= (const timer &t1, const timer &t2);
	friend class timerQueue;


};

#endif
