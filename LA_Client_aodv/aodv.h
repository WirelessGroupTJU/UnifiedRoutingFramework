/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/


#ifndef	_AODV_H_
#define _AODV_H_


#include "main.h"
#include "aodvSocket.h"
#include "sendData.h"
#include "rerr.h"

typedef void (*hfunc_t) (void);

class aodv{

	aodvSocket	aodvSock;  /* aodv socket object */

	/* array to store handler functions for aodv and asl sockets */
	struct handler{
		int fd;
		hfunc_t	func;
	}handlers[MAX_HANDLERS];

	int	handler_cnt;

	public:

	aodv()
	{
		handler_cnt = 0;
	}

	void	aodv_init(char *interface);
	int		aodv_daemon(char *interface);

	int		registerSignal();
	void		sendHello();

	void	rrep_ack_handler(void *data);

	void	register_handler_func(int fd, hfunc_t func);
	void	process_aodv(void);

	void	handler(int type);
	void	sigint_handler(int type);
	
	void	genSndData(sendData *snd, u_int32_t dst, int ttl);
	void 	processRecvMsg(struct aodvData *data, int len);
	void	processRREQ(aodvData *data);
	void	processRREP(aodvData *data);
	void	processRERR(aodvData *data);
	void	processRREP_ACK(aodvData *data);

	u_int32_t	get_next_hop(u_int32_t dst);
	void		recv_asl_data();
	//zyp
	void		recv_la_data();
	void		process_asl(struct rt_info *rInfo);
	void		process_asl(struct route_info *rInfo);
	void		packBuf(char *buf, RERR *rerrOb);
	int			doLocalRepair(u_int32_t dst, u_int32_t src, u_int8_t rflags);
	int			sendRREQ(u_int32_t dst_ip, u_int8_t flags);
	int			setRREQTimer(u_int32_t dst, int ttl);
	int			setLRTimer(u_int32_t dst, int ttl);
	void		LR_handler(void * data);
	void		resendRREQ(void *val);
	void		sendPacket(sendData *snd, void *data, int data_len);
	void		copyToSendBuf(void *data, int len);
	void		mainDaemonLoop();
	void 		generateRERR(u_int32_t dst, u_int8_t N_flag, bool brdFlag);

	void		setRebootTimer();
	void		setPeriodicTimer();
	void		rebootHandler();
	void		periodicHandler();
};

#endif
