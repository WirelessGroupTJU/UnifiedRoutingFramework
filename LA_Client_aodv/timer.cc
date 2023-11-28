/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#include "common.h"


bool operator< (const timer &t1, const timer &t2)
{

	return t1.timeout < t2.timeout;
}
bool operator<= (const timer &t1, const timer &t2)
{

	return t1.timeout <= t2.timeout;
}

bool operator> (const timer &t1, const timer &t2)
{

	return t1.timeout > t2.timeout;
}
bool operator>= (const timer &t1, const timer &t2)
{

	return t1.timeout >= t2.timeout;
}

bool operator== (const timer &t1, const timer &t2)
{

	return t1.timeout == t2.timeout;
}

bool operator != (const timer &t1, const timer &t2)
{

	return t1.timeout != t2.timeout;
}
