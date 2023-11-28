/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef	RREPACK_H
#define RREPACK_H

#include "const.h"

class RREP_ACK{

	u_char	type:8;
	u_char	reserved:8;

	public:

	RREP_ACK()
	{
		type = AODV_RREP_ACK;
		reserved = 0;
	}

};

#endif
