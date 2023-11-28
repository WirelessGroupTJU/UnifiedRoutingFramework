#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <sys/types.h>
#include "nd_timer.h"
#include "nd_type.h"

typedef uint32_t u_int32_t;
typedef uint64_t u_int64_t;

class timer{

	uint64_t	timeout;
	//struct timeval	timeout;
	timer_hfunc_t		handler;
	void			*data;

	public:

	timer() {}

	timer(uint64_t timer_val, timer_hfunc_t handler_ptr, void *func_data)
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
