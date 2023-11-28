/* this file implements the data structure for storing the route cache.
* It is basically a linked list.
* This code has been modified from the flood_id_queue.c of NIST's kernel-AODV
* implementation by Luke Klein-Berndt.
* --Vikas Kawadia May 19,2002
*/  


#ifndef ROUTE_CACHE_H
#define ROUTE_CACHE_H

#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/skbuff.h>
#include <linux/in.h>


#include "utils.h"

/* an entry of the route cache */
struct route_cache_entry
{
    u_int32_t		   dst_ip;      // destination ip of the packet
    u_int64_t          last_use_time; // time route was last used
	int				   valid_flag;
    struct route_cache_entry *prev;
    struct route_cache_entry *next;
};

/* initialize the cache */
int init_route_cache( void );

/* find an entry based on dst_ip  */
// needed to update the last_use_time whenever needed
struct route_cache_entry *find_route_cache_entry(u_int32_t dst_ip, int vFlag);

/* print the cache to system logs */
void print_route_cache(void);

/* you're right. free memory, clenaup etc */
void cleanup_route_cache(void);

/* insert a new entry in to the cache */
int insert_route_cache_entry(u_int32_t dst_ip, u_int64_t last_use_time, int vFlag);

/* output the stuff to /proc. Called when the proc file is read by somebody from userspace */
ssize_t	read_route_cache_proc(struct file *file,  char __user *user_buf,    size_t buffer_length,   loff_t *offset);

/*
Route cache keeps on growing. So we need to keep a check on the size.
This function deletes the entries whose last_use_time is older than 
EXPIRE_TIME defined in module.h.
I dont know when to call this function. will think about it. --vikas
FIXME
*/
void delete_old_route_cache_entries(void );

#endif







