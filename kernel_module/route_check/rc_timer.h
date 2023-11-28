
#ifndef RC_TIMER_H
#define RC_TIMER_H

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/signal.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/random.h>

void rc_timer_handler(struct timer_list *test);

#endif
