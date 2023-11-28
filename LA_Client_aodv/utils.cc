/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"


/* Check for root preiviledge */
int check_root(void){
	  if (getuid() || geteuid())
	    return -1;

	  return 0;
}

/* Initialize socket for sending dsdv control messages */
int init_socket (char *IF)
{
	  int fd;
	  int on = 1;
	  int b_cast = 1;
	  int type = SOCK_DGRAM;
	  int zero=0;

	  /* Get socket for UDP or TCP */
	  if ((fd = socket(AF_INET, type, 0)) == -1)
	    /* Error creating socket */
	    return -1;

	  /* Setting socket to only listen to out device (and not loopback) */
	  if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, IF,
			 (size_t)((strlen(IF)+1)*sizeof(char))) < 0)
	    /* Error in setting socket */
	    return -1;

	  /* Setting socket so that more than one process can use the address */
	  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	    /* Error in setting socket */
	    return -1;

	  /* Listen to broadcast as well? */
	  if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &b_cast, sizeof(b_cast)) < 0)
	    /* Error in setting socket */
	    return -1;

	if (setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &zero, sizeof(zero)) < 0)
	  {
		  perror("Error Setting priority");
	      /* Error in setting socket */
	      return -1;
	  }
	  return fd;
}

/* bind dsdv socket to DSDVPORT */
int bind_socket (int fd, struct sockaddr_in *addr)
{
	  addr->sin_family = AF_INET;
	  addr->sin_port = htons(AODV_PORT);
	  bzero(&(addr->sin_zero), 8);
	  addr->sin_addr.s_addr=INADDR_ANY;

	  /* bind aodv socket to my address */
	  if (bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr)) == -1)
	    /* Error binding socket */
	    return -1;

	  return 0;
}


/* This sets the global variable g_my_ip to the interface ip address
 * at which dsdv protocol is running */
int get_interface_ip (int fd, char *IF, struct sockaddr_in *addr)
{
	  struct sockaddr *sockaddrp;
	  struct ifreq  *interfacep;
	  struct sockaddr_in *addrp;
	  struct ifconf ifc;
	  char buf[500];
	  int n;

	  /* Get interface configuration */
	  ifc.ifc_len = sizeof (buf);
	  ifc.ifc_buf = buf;
	  if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) < 0)
	    /* Error in ioctl */
	    return(-1);

	  interfacep = ifc.ifc_req;

	  /* Parse config. for all interfaces */
	  for (n = ifc.ifc_len / sizeof (struct ifreq); --n >= 0; interfacep++)
	    {

	      if (interfacep->ifr_addr.sa_family != AF_INET)
		continue;

	      sockaddrp = &(interfacep->ifr_addr);
	      addrp = (struct sockaddr_in*)sockaddrp;

	      if (strncmp(interfacep->ifr_name, IF, strlen(IF)) == 0)
			addr->sin_addr = addrp->sin_addr;
		else
			continue;
	    }

	  /* Sets global variable g_my_ip */
	  g_my_ip = addr->sin_addr.s_addr;


	 return (0);
}

/* Return current time in msecs */
u_int64_t getcurrtime()
{
	  struct timezone tz;
	  struct timeval tv;
	 
	  if (gettimeofday(&tv, &tz) < 0)
	    /* Couldn't get time of day */
	    return 0;

	  //return ((u_int32_t)tv.tv_sec) * 1000 + ((u_int32_t)tv.tv_usec) / 1000;
	  return ((u_int64_t)tv.tv_sec) * 1000 + ((u_int64_t)tv.tv_usec) / 1000;
}

char * getDotIP(u_int32_t addr)
{
	struct in_addr ip_addr;
	ip_addr.s_addr = addr;
	return inet_ntoa(ip_addr);
}

/* This initializes socket for communication with the kernel */
int init_krtsocket()
{

	  int fd = 0;
	  int type = SOCK_DGRAM;

	  /* Get socket for UDP or TCP */
	  if ((fd = socket(AF_INET, type, 0)) < 0)
	  {
		  perror("Error in initializing kernel socket");
	  	  return -1; /* Unable to create socket */
	  }
	  
#ifdef DEBUG
	  cout << "Initialized kernel Socket" <<  fd << endl;
#endif

	  return fd;
}

