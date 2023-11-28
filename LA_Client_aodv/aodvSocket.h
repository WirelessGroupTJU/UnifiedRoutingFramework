/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _AODVSOCKET_H_
#define _AODVSOCKET_H_

#include "main.h"

class aodvSocket{

	int sock;	/* socket descriptor */

	
	public:

	int	getSock() { return sock; }

	int	init_socket(char *interface);
	int	bind_socket(int fd, struct sockaddr_in *addr);

	int	createSock(char *interface, struct sockaddr_in *addr);

	int	readFromSock(struct aodvData *data);

};

#endif
