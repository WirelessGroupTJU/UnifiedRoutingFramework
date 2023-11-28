/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _MAIN_H
#define _MAIN_H

/* this structure is used to identify the type of received aodv message */
typedef struct {
	u_int8_t type;
}AODV_Msg;

struct aodvData {
	u_int32_t	dest_ip;
	u_int32_t	src_ip; /* prev hop from where the message was received */
	int			ttl; 
};

struct timerData {
	int				type;
	u_int32_t		data;
};

#endif
