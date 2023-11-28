/* this file implements the data structure for storing the route cache.
* It is basically a linked list.
* This file has been modified from the flood_id_queue.c of NIST's kernel-AODV
* implementation by Luke Klein-Berndt.
*
* --Vikas Kawadia May 19,2002
*/  

#include "route_cache.h"
#include "module_main.h"
#define DEBUG9

struct route_cache_entry *route_cache; //链表头

int init_route_cache( void )
{
#ifdef DEBUG9
	printk(KERN_INFO "in init route_cache()");
#endif
    route_cache=NULL;
    return 0;
}

/****************************************************

   find_route_cache_entry
----------------------------------------------------
will search the queue for an entry with the
matching dst_ip  链表查找
****************************************************/
struct route_cache_entry *find_route_cache_entry(u_int32_t dst_ip, int vFlag)
{
    struct route_cache_entry  *tmp_entry;  /* Working entry */

    tmp_entry = route_cache; /* Start at the header */
#ifdef DEBUG9
	printk(KERN_CRIT "in find_route_cache route_cache()");
#endif

    //go through the whole queue
    while (tmp_entry!=NULL)
    {
        //if there is a match and it is still valid
        if ((dst_ip == tmp_entry->dst_ip) && (vFlag == tmp_entry->valid_flag))
                return tmp_entry;

        //continue on to the next entry
        tmp_entry=tmp_entry->next;

    }
    return NULL;
}




/****************************************************

   read_route_cache_proc
----------------------------------------------------
output the stuff to /proc. Called when the proc file is read by somebody from userspace 

****************************************************/

ssize_t	read_route_cache_proc(struct file *file,  char __user *user_buf,    size_t buffer_length,   loff_t *offset)
{
    int len ;
    unsigned long p =  *offset;
    unsigned int count = buffer_length;
    ssize_t ret = 0;
    struct route_cache_entry *tmp_entry; 

    char *my_buffer =(char *)kmalloc(buffer_length,GFP_ATOMIC);
    char temp_buffer[80]; 	memset(temp_buffer,0,sizeof(temp_buffer));

    tmp_entry=route_cache;

    sprintf(my_buffer,"-----------Route Cache---------------------------------\n");
    sprintf(my_buffer,"Destination\t Last_use_time(In msec)\t Valid Flag\n");

    while (tmp_entry!=NULL)
    {

        sprintf(temp_buffer,"%s\t %lu\t\t %d\n", inet_ntoa(tmp_entry->dst_ip),(unsigned long)(tmp_entry->last_use_time), tmp_entry->valid_flag);
        strcat(my_buffer,temp_buffer);
        tmp_entry=tmp_entry->next;
    }


    len= strlen(my_buffer);
    /*分析和获取有效的写长度*/
    if (p >= len)
        return count ?  - ENXIO: 0;
    if (p + count > len )   
        count = len - p;

    /*内核空间->用户空间*/
    if (copy_to_user(user_buf, (void*)(my_buffer+p), count))
    {
        ret =  - EFAULT;
    }
    else
    {
        *offset += count;
        ret = count;
        printk(KERN_INFO "read %u bytes(s) from %lu\n", count, p);
    }

    kfree(my_buffer);
    return ret;

}


/****************************************************

   print_route_cache
----------------------------------------------------
prints out the route cache onto the console screen
****************************************************/
void print_route_cache(void)
{
    struct route_cache_entry *tmp_entry;
    tmp_entry=route_cache;

    printk("-----------Route Cache---------------------------------\n");
    printk("Destination \t Source_IP (prev hop) \t Last_use_time (In msec, since the Epoch)\n");
    while (tmp_entry!=NULL)
    {
        printk("%s \t %lu\t %d\n", inet_ntoa(tmp_entry->dst_ip),(unsigned long)(tmp_entry->last_use_time), tmp_entry->valid_flag);
        tmp_entry=tmp_entry->next;
    }
}

/****************************************************

   clean_up_route_cache
----------------------------------------------------
Deletes everything in the route cache
****************************************************/
void cleanup_route_cache(void)
{
    struct route_cache_entry *tmp_entry,*dead_entry;
    int count=0;

    tmp_entry=route_cache;

    while (tmp_entry!=NULL)
    {
        dead_entry=tmp_entry;
        tmp_entry=tmp_entry->next;
        kfree(dead_entry);
        count++;
    }

}

/****************************************************

   insert_route_cache_entry
----------------------------------------------------
Inserts an entry into the route cache 链表头插法
****************************************************/
int insert_route_cache_entry(u_int32_t dst_ip, u_int64_t last_use_time, int vFlag)
{
    struct route_cache_entry  *new_entry; /* Pointer to the working entry */

    /* The header wasn't empty, find the last entry ??? */

    /* Allocate memory for the new entry */
    if ((new_entry = (struct route_cache_entry*)kmalloc(sizeof(struct route_cache_entry),GFP_ATOMIC)) == NULL)
    {
#ifndef NO_ERROR
        printk("INSERT_route_cache_ENTRY: Error creating route cache entry\n");
#endif
        /* Failed to allocate memory for new Route Request */
        return 1;
    }
    /* Fill in the information in the new entry */
    new_entry->dst_ip = dst_ip;
    new_entry->last_use_time = last_use_time;
    new_entry->valid_flag = vFlag;
    new_entry->next = route_cache;

    /* Put the new entry in the list */
    route_cache=new_entry;

    return 0;
}

/****************************************************

   delete_old_route_cache_entries
----------------------------------------------------
Route cache keeps on growing. So we need to keep a check on the size.
This function deletes the entries whose last_use_time is older than 
EXPIRE_TIME defined in module.h.
I dont know when to call this function. will think about it. --vikas
****************************************************/
void delete_old_route_cache_entries(void )
{
    struct route_cache_entry *tmp_entry,*prev_entry,*dead_entry;
    u_int64_t  curr_time = getcurrtime(); /* Current time */


    tmp_entry=route_cache;
    prev_entry=NULL;

    //go through the entire queue
    while(tmp_entry!=NULL)
    {

        //if the entry has expired
        if (curr_time > tmp_entry->last_use_time + EXPIRE_TIME)
        {
            //if it is the first entry
            if(prev_entry==NULL)
                route_cache=NULL;
            else
                prev_entry->next=tmp_entry->next;

            //kill it!
            dead_entry=tmp_entry;
            tmp_entry=tmp_entry->next;
            kfree(dead_entry);
        }
        else
        {
            //next entry
            prev_entry=tmp_entry;
            tmp_entry=tmp_entry->next;
        }
    }
}

/****************************************************

   delete_route_cache_entry
----------------------------------------------------
Deletes a route cache queue entry
****************************************************/
int delete_route_cache_entry(struct route_cache_entry *dead_entry)
{
    struct route_cache_entry *tmp_entry, *prev_entry;

    tmp_entry=route_cache;
    prev_entry=NULL;

    //since we don't know the previous entry we have to go
    //through entire list to find it!
    while (tmp_entry!=NULL)
    {
        //if we find the entry we wish to delete
        if(tmp_entry==dead_entry)
        {
            if (prev_entry==NULL)
                route_cache=NULL;
            else
                prev_entry->next=tmp_entry->next;
            kfree(tmp_entry);
            return 0;
        }
        tmp_entry=tmp_entry->next;
    }
    return 1;
}



