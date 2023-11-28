/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/



#ifndef SENDDATA_H
#define SENDDATA_H

#include "parameters.h"

class sendData
{
	u_int32_t	dest_ip;
	int			ttl;

	public:

	sendData()
	{
		ttl=NET_DIAMETER;
	}

	u_int32_t	getDestIP() { return dest_ip;}
	int			getTTL() { return ttl;}

	void	setDestIP(u_int32_t dst) { dest_ip = dst; }
	void 	setTTL(int tl) { ttl = tl; }
};

#endif
