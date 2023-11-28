
#include "rc_timer.h"
#include "module_main.h"
#include "route_cache.h"
#include <linux/stddef.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/fs.h>

struct timer_list rc_timer ;
 
int sync_it(void);

void rc_timer_handler(struct timer_list *test)
{

	/* Sync with the kernel routing table */
	sync_it();
	
	/* set the timer off again */
	rc_timer.expires = jiffies + (EXPIRE_TIME * HZ);
	//rc_timer.data = 0;
	rc_timer.function = rc_timer_handler ;
	add_timer(&rc_timer);
}


int
sync_it(void)
{
//	int fd;
//	fd = open("/proc/net/route", O_RDONLY);
//	printk(KERN_INFO "route_check.o : sync_it() \n");
//	close(fd);
	
	return 0;
}
