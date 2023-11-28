/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _UTILS_H_
#define _UTILS_H_

extern int check_root(void);
extern int init_socket (char *IF);
extern int bind_socket (int fd, struct sockaddr_in *addr);
extern int get_interface_ip (int fd, char *IF, struct sockaddr_in *addr);
extern u_int64_t	getcurrtime();
extern char *		getDotIP(u_int32_t addr);
extern	int			init_krtsocket();

#endif
