/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"
#include "externDec.h"

/* create a new socket */
int	aodvSocket::createSock(char *interface, struct sockaddr_in *addr)
{

	sock = init_socket(interface);
	if(sock < 0)
	{
		cout << "Error creating aodv socket"  << endl;
		return -1;
	}

	if( bind_socket(sock,addr) < 0)
		return -1;

	return 0;

}

/* read data from the aodv socket */
int	aodvSocket::readFromSock(struct aodvData *data)
{
	//AODV_Msg 	*aodv_msg;
	int	len;
	    struct sockaddr_in src_addr;
	    struct msghdr msg;

	    union control_union{
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int)) +
			     CMSG_SPACE(sizeof(struct in_pktinfo))];
	    };

		union control_union control_un;

	    struct cmsghdr *cmsg;
		char	*cmsgPtr;

	    struct in_pktinfo pktinfo;
	    socklen_t	addr_len;
	    int	recvTTL=0;

		memset(&src_addr,0,sizeof(struct sockaddr_in));

	    msg.msg_name = NULL;
	    msg.msg_namelen = 0;
	    msg.msg_iov = NULL;
	    msg.msg_iovlen = 0;
	    msg.msg_control = control_un.control;
	    msg.msg_controllen = sizeof(control_un.control);

	    /* Get the information control message first */
	    if ((len = recvmsg(sock, &msg, MSG_PEEK)) < 0) {
			cout << "Error in recvmsg " << endl;
			return -1;
	    }
		addr_len = sizeof(struct sockaddr_in);

	    /* Read the data payload (i.e. AODV msg) */
	    len = recvfrom(sock, recvBuffer, AODVBUFSIZE, 0,
			   (struct sockaddr *) &src_addr, &addr_len);

	    if (len < 0) {
		    cout << "aodv socket: error in recvfrom " << endl;
			return -1;
	    }

	    u_int32_t	src = (src_addr.sin_addr.s_addr);

#ifdef DEBUG
		cout << " readFromSock: src address is " << getDotIP(src) << endl;
		cout << " readFromSock: local node address is " << getDotIP(g_my_ip) << endl;
#endif
		data->src_ip = src;
	    /* ignore all messages received from the local node */
	    if(src == g_my_ip)
		    return 0;

	    

        cmsg = CMSG_FIRSTHDR(&msg);
		printf("controllen is %d\n", msg.msg_controllen);
	    if(msg.msg_controllen == 56) {
		for (int i = 0; i < 2; i++) {
			if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_TTL) {
			    memcpy(&recvTTL, CMSG_DATA(cmsg), sizeof(int));
				cmsgPtr = (char*)cmsg;
			    cmsgPtr =  cmsgPtr + CMSG_SPACE(sizeof(int));
				cmsg = (struct cmsghdr *)cmsgPtr;
			} else if (cmsg->cmsg_level == SOL_IP
				   && cmsg->cmsg_type == IP_PKTINFO) {
			    memcpy(&pktinfo, CMSG_DATA(cmsg), sizeof(struct in_pktinfo));
				cmsgPtr = (char*)cmsg;
			    cmsgPtr =  cmsgPtr + CMSG_SPACE(sizeof(struct in_pktinfo));
				cmsg = (struct cmsghdr *)cmsgPtr;
			}
		}
        }
		
		else
			cout <<" read From AODV socket: header not found " << endl;

#ifdef DEBUG
		cout << "readFromSocket: ttl received is " << recvTTL << endl;
#endif

	data->dest_ip = (pktinfo.ipi_addr.s_addr);
	data->ttl = (recvTTL);

#ifdef DEBUG
	cout << " readFromSock: src address is " << getDotIP(data->src_ip) << endl;
	cout << " readFromSock: dest address is " << getDotIP(data->dest_ip) << endl;
	cout << " readFromSock: local node address is " << getDotIP(g_my_ip) << endl;
	cout << " readFromSock: ttl is " << data->ttl << endl;
#endif

	return len;

}

/* initialize aodv socket */
int 	aodvSocket::init_socket (char *IF)
{
	  int fd;
	  int on = 1;
	  int b_cast = 1;
	  int	pri=0;
	  int type = SOCK_DGRAM;

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

	  /* set socket priority */
	  if (setsockopt(fd, SOL_SOCKET, SO_PRIORITY , &pri, sizeof(pri)) < 0)
		  return -1;

	  /* set option to receive IP_TTL for received messages. */
  	  if (setsockopt(fd, SOL_IP, IP_RECVTTL, &on, sizeof(on)) < 0)
		return -1;

  	  /* set option to receive PKTINFO for received messages. */
  	  if (setsockopt(fd, SOL_IP, IP_PKTINFO, &on, sizeof(on)) < 0)
		return -1;

	  return fd;
}

int 	aodvSocket::bind_socket (int fd, struct sockaddr_in *addr)
{
	  addr->sin_family = AF_INET;
	  addr->sin_port = htons(AODV_PORT);
	  bzero(&(addr->sin_zero), 8);
	  addr->sin_addr.s_addr=INADDR_ANY;

	  /* bind aodv socket to my address */
	  if (bind(fd, (struct sockaddr *)addr, sizeof(struct sockaddr)) == -1)
	    return -1;

	  return 0;
}


