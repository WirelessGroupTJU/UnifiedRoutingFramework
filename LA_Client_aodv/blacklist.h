/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef _BLACKLIST_H_
#define _BLACKLIST_H_


/* an entry of blacklist */
class blacklist_entry {
	u_int32_t	dest_ip;
	u_int64_t	lifetime;

	public:

	u_int32_t	getDestIP() { return dest_ip; }
	u_int64_t	getLifeTime() { return lifetime; }

	void		setDestIP(u_int32_t dst) { dest_ip = dst; }
	void		setLifeTime(u_int64_t lTime) { lifetime = lTime; }
};

/* class implementing black list for aodv */
class blacklist{

	list<blacklist_entry>	bList;

	public:

	void	addToList(u_int32_t dest, u_int64_t lTime);
	bool	inList(u_int32_t dest);
	void	updateEntries();

};


#endif
