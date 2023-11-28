/* this file contains values of different AODV parameters */
#ifndef PARAM_H
#define PARAM_H

#define	AODVBUFSIZE 32*1024

#define	NODE_TRAVERSAL_TIME		500
#define ALLOWED_HELLO_LOSS		2
#define	HELLO_INTERVAL			2000  /* ms */
#define	PERIODIC_INTERVAL		1000  /* ms */
#define	ACTIVE_ROUTE_TIMEOUT	3000  /* ms */
#define	RREP_ACK_TIMEOUT		NODE_TRAVERSAL_TIME+20  /* ms */

#define	BLACKLIST_TIMEOUT		NET_TRAVERSAL_TIME * (((TTL_THRESHOLD - TTL_START)/TTL_INCREMENT)+1+RREQ_RETRIES)

#define LOCAL_ADD_TTL			2
#define TTL_START				1
#define	TTL_INCREMENT			2
#define	TTL_THRESHOLD			7

#define	NET_DIAMETER			35

#define	RREQ_RETRIES			2	

#define K					    5	
#define DELETE_PERIOD			K * MAX(ACTIVE_ROUTE_TIMEOUT, HELLO_INTERVAL)

#define	INFINITY				60
#define	MAX_REPAIR_TTL			0.3 * NET_DIAMETER

#define	MY_ROUTE_TIMEOUT		10 * ACTIVE_ROUTE_TIMEOUT

#define	NET_TRAVERSAL_TIME		3*NODE_TRAVERSAL_TIME*NET_DIAMETER/2
#define	PATH_TRAVERSAL_TIME		2 * NET_TRAVERSAL_TIME

#endif

