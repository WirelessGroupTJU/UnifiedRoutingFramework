/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
************************************************************************/
#include <sys/select.h> 
// #include <libexplain/select.h>


#include "common.h"
#include "externDec.h"

void	process_aodv_handler(void);
//老版本ASL的处理函数
void	recv_asl_data_handler(void);
//新版本处理客户端消息的函数
void	recv_la_data_handler(void);

void	resendRREQ_handler(void * data);
void	LR_timer_handler(void *data);
void	global_handler(int type);
void	sigint_timer_handler(int type);
void	sendHello_handler(void * data);
void	rrepAck_handler(void * data);
void	rebootHandler_stub(void *data);
void	periodicHandler_stub(void *data);

/* AODV initialization function */
void	aodv::aodv_init(char * interface)
{
	struct sockaddr_in my_addr;
	struct in_addr	def;

	if(check_root() == -1)
	{
		cout<< "You must be root to run this program" << endl;
		exit(1);
	}

	/* create AODV socket and register handler function */
	aodvSock.createSock(interface, &my_addr);//This sock is for a UDP server port:645
	register_handler_func(aodvSock.getSock(), process_aodv_handler);

	if (get_interface_ip(aodvSock.getSock(), interface, &my_addr) == -1)
	{
	      cout << "Error getting Interface IP" << endl;
	      exit(1);
	}

	// /* ASL: open a new socket connection to asl thread */
	// asl_sock = open_route_request();
	// if(asl_sock < 0)
	// 	cout << "Failed to open route request " << endl;
	// register_handler_func(asl_sock, recv_asl_data_handler);

	// /* ASL: add a deferred route */
	// inet_aton("0.0.0.0",&def);
  	// if (route_add(def.s_addr, def.s_addr, interface) < 0)
	// 	cout << "Error in adding deferred route" << endl; 

	// /* open socket for communication with kernel routing table */
	// ksock = init_krtsocket();
	// if(ksock == -1)
	//   {	
	// 	cout << "Error Initializing kernel socket" << endl;
	// 	exit(1);
	//   }
	
	//在这份AODV代码中需要添加fo模块，以及注册为LA的客户端
	//step1：注册fo模块
	int fo_ret = init_fo_socket();
	if(fo_ret < 0){
		printf("[fo] module load failed!\n");
		exit(1);
	}else {
		printf("[fo] module load successfully!\n");
	}

	//step2:注册成为LA的客户端

    struct sockaddr_in servaddr;

    /* 创建套接字 */
    if((la_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket eror");

    /* 构建结构体ip、port */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0)
        printf("inet_pton error");

    /* 建立连接 */
    int c = connect(la_sock, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    if(c == -1) {
        printf("connnect error\n");
    }
	register_handler_func(la_sock, recv_la_data_handler);

	return;
}

void	aodv::register_handler_func(int fd, hfunc_t fun)//-ok
{
	handlers[handler_cnt].fd = fd;
	handlers[handler_cnt].func = fun;
	handler_cnt++;

	return;
}

/* handler function for aodv socket */
void	process_aodv_handler(void)	//-OK
{
	aodvOb.process_aodv();

}

/* this function is called when data is received on aodv socket */
void	aodv::process_aodv(void)	//-OK
{
	/* receive aodv message and call appropriate handlers */
	struct	aodvData	data;
	int	length;

	memset(&data,0,sizeof(struct aodvData));

	length = aodvSock.readFromSock(&data);
	processRecvMsg(&data, length);
	return;
}

void	aodv::genSndData(sendData *snd,u_int32_t dst, int ttl)
{
	snd->setDestIP(dst);
	snd->setTTL(ttl);
}

/* This function processes received AODV messages */
void	aodv::processRecvMsg(struct aodvData *data, int len)	//处理从sock接收到的data
{
	AODV_Msg	*aodv_msg;

	aodv_msg = (AODV_Msg *)recvBuffer;

#ifdef DEBUG
	cout << "processRecvMsg:: entered" << endl;
	cout << " processRecvmsg: src address is " << getDotIP(data->src_ip) << endl;
	cout << " processRecvMsg: local node address is " << getDotIP(g_my_ip) << endl;
#endif

	if(data->src_ip == g_my_ip)
	{
		cout << "processRecvMsg: ignore messages from the local node" << endl;
		return;
	}
#ifdef DEBUG
	cout << "processRecvMsg: received ttl is " << data->ttl << endl;
#endif
	/* decrement ttl value */
	data->ttl--;

	/* check for various received message types and do
	 * processing accordingly */

	switch(aodv_msg->type){
		case AODV_RREQ:
			processRREQ(data);
			break;
		case AODV_RREP:
			processRREP(data);
			break;
		case AODV_RERR:
			processRERR(data);
			break;
		case AODV_RREP_ACK:
			processRREP_ACK(data);
			break;
		default:
			cout << "processRecvMsg: default case" << endl;
			break;
	}
}

void		aodv::processRREQ(aodvData *data)
{
	int result=0;
	AODV_Msg	*aodv_msg;
	sendData	snd;
	u_int32_t	next_hop;
	u_int64_t	currtime;

	aodv_msg = (AODV_Msg *)recvBuffer;

#ifdef DEBUG
	cout << "aodv: processRREQ: entered" << endl;
#endif

	if(duringReboot)
	{
#ifdef DEBUG
		cout << "aodv: processRREQ: in the reboot process" << endl;
#endif
		/* create route entries for the prev hop and the source
		 * of the rreq */

		RREQ	rreqOb((RREQ *)aodv_msg);
		rtable_entry	rtEntry, rtNeighborEntry;
		u_int32_t		seqNum=0;

		currtime = getcurrtime();

		if(rreqOb.getSrcIP() == data->src_ip)
			seqNum = rreqOb.getSrcSeqNum();

		/* update route to the neighbor */
		rtable.rebootNeighborUpdate(data->src_ip,seqNum,(currtime+ALLOWED_HELLO_LOSS*HELLO_INTERVAL));

		/* update reverse route */
		if(rreqOb.getSrcIP() != data->src_ip)
			rtable.rebootReverseRouteUpdate(data->src_ip, &rreqOb);

		/* update the local nodes seq number from the received packet */
		if(rreqOb.getDestIP() == g_my_ip)
		{
			if(rreqOb.getDestSeqNum() > localSeqNum)
				localSeqNum = rreqOb.getDestSeqNum();
		}
		return;
	} 
	else
	{

#ifdef DEBUG
		cout << "aodv: processRREQ: not inside reboot " << endl;
#endif

		RREQ	rreqOb((RREQ *)aodv_msg);

#ifdef DEBUG
		cout << "aodv: processRREQ: dest ip is " << getDotIP(rreqOb.getDestIP())<< endl;
		cout << "aodv: processRREQ: hop cnt is " << (int)(rreqOb.getHopCnt())<< endl;
		cout << "aodv: processRREQ: dest seq num is" << (rreqOb.getDestSeqNum())<< endl;
		cout << "aodv: processRREQ: src ip is" << getDotIP(rreqOb.getSrcIP())<< endl;
		cout << "aodv: processRREQ: src seq num is" << (rreqOb.getSrcSeqNum())<< endl;
		cout << "aodv: processRREQ: rreq id is" << (rreqOb.getRreqId())<< endl;
#endif

		/* process received RREQ */
		result = rreqOb.recvRREQ(data->src_ip);
		if(result == 0)
		{
			/* do nothing */
			return;
		}
		else if( result == FWD_RREQ)
		{
#ifdef DEBUG
			cout << "aodv: processRREQ: forward the RREQ " << endl;
			cout << "aodv: processRREQ: ttl value is " << data->ttl << endl;
#endif
			if(data->ttl > 0)
			{
				genSndData(&snd, BROADCAST_ADDR, data->ttl);
				copyToSendBuf((void *)&rreqOb, sizeof(RREQ));
				sendPacket(&snd,(void *)sendBuffer,sizeof(RREQ));
			}
		}
		else if(result == GEN_RREP)
		{
#ifdef DEBUG
			cout << "aodv: processRREQ: generate RREP " << endl;
#endif
			/* generate a RREP packet */
			RREP	rrepOb;
			rrepOb.createRREP(data->src_ip,(RREQ *)aodv_msg);

			if((next_hop = get_next_hop(rreqOb.getSrcIP())) != 0)
			{

				/* unicast RREP */
				/* also set up a rrep_ack timer if needed*/
				if(rrepOb.getA() == 1)
				{
#ifdef DEBUG
					cout << "aodv::processRREQ: set rrep_ack timer " << endl;
#endif
					struct timerData	tData;
					tData.type = RREP_ACK_TIMER;
					tData.data = next_hop;
					timer_Q.set_timer(RREP_ACK_TIMEOUT, rrepAck_handler, (void *) &tData);
				}

				genSndData(&snd, next_hop, -1);
				copyToSendBuf((void *)&rrepOb, sizeof(RREP));
				/* send out the generated packet */
				sendPacket(&snd,(void *)sendBuffer,sizeof(RREP));
			}

			/* check if need to generate gratuitour RREP */
			if(rreqOb.getG() == 1 && (rreqOb.getDestIP() != g_my_ip))
			{
				/* Generate Gratuitous RREP at the intermediate node*/
#ifdef DEBUG
				cout << "aodv: processRREQ: generate gratuitous RREP " << endl;
#endif
				RREP	grrepOb;
				grrepOb.createGRREP(rreqOb.getSrcIP(), rreqOb.getDestIP(), rreqOb.getSrcSeqNum());
				if((next_hop = get_next_hop(rreqOb.getDestIP())) != 0)
				{
					genSndData(&snd, next_hop, -1);
					copyToSendBuf((void *)&grrepOb, sizeof(RREP));
					sendPacket(&snd,(void *)sendBuffer,sizeof(RREP));
				}
			}
		}
	}
	return;
}

/* handler function called when timer to receive RREP-ACK expires */
void	rrepAck_handler(void *data)
{
	aodvOb.rrep_ack_handler(data);
}


void	aodv::rrep_ack_handler(void *val)
{
	/* add the next hop to the blacklist of nodes */
	struct timerData	*tData;

#ifdef DEBUG
	cout << "aodv::rrep_ack_handler: entered" << endl;
#endif

	tData = (struct timerData*)val;
	u_int32_t	dst=tData->data;

	/* add the node to the black list */
	black_list.addToList(dst, BLACKLIST_TIMEOUT);
}

/* this function processes incoming RREP */
void		aodv::processRREP(aodvData *data)
{
	int result=0;
	AODV_Msg	*aodv_msg;
	sendData	snd;
	u_int32_t	next_hop;
	u_int64_t	currtime;

	aodv_msg = (AODV_Msg *)recvBuffer;

// #ifdef DEBUG
	cout << "aodv: processRREP: entered" << endl;
// #endif

	if(duringReboot)
	{
// #ifdef DEBUG
	    cout << "aodv: processRREP: during reboot time - ignore received RREP" << endl;
// #endif

		/* create route entries for the prev hop and the source
		 * of the rrep */
		RREP	rrepOb((RREP *)aodv_msg);
		rtable_entry	rtEntry;
		u_int32_t		seqNum=0;

		currtime = getcurrtime();

		if(rrepOb.getDestIP() == data->src_ip)
			seqNum = rrepOb.getDestSeqNum();

		/* update route to the neighbor */
		rtable.rebootNeighborUpdate(data->src_ip,seqNum,(currtime+ALLOWED_HELLO_LOSS*HELLO_INTERVAL));
		/* update forward route */
		if(rrepOb.getDestIP() != data->src_ip)
			rtable.rebootForwardRouteUpdate(data->src_ip, &rrepOb);
		return;
	}
	else
	{
// #ifdef DEBUG
	    cout << "adov: processRREP: not during reboot time" << endl;
// #endif
	        /* generate a RREP object from the received buffer */
		RREP	rrepOb((RREP *)aodv_msg);

		/* check if an ACK needs to be sent for the received RREP */
		if( rrepOb.getA() == 1)
		{
			/* send a rrep_ack message */
			RREP_ACK		rrepAckOb;
// #ifdef DEBUG
	    	cout << "adov: processRREP: rrep ack flag set, send ack" << endl;
// #endif

			genSndData(&snd, data->src_ip, 1 );
			copyToSendBuf((void *)&rrepAckOb, sizeof(RREP_ACK));
			sendPacket(&snd,(void *)sendBuffer,sizeof(RREP_ACK));
		}

		/* process the received RREP */
		result = rrepOb.recvRREP(data->src_ip);
		if(result == SEND_RREP)
		{
// #ifdef DEBUG
	    	cout << "aodv: processRREP: send RREP ahead" << endl;
// #endif

			if((next_hop = get_next_hop(rrepOb.getSrcIP())) != 0)
			{
				/* also set up a rrep_ack timer if needed*/
				if(rrepOb.getA() == 1)
				{
					struct timerData	tData;
					tData.type = RREP_ACK_TIMER;
					tData.data = next_hop;
					timer_Q.set_timer(RREP_ACK_TIMEOUT, rrepAck_handler, (void *) &tData);
				}

				genSndData(&snd, next_hop, -1 );
				copyToSendBuf((void *)&rrepOb, sizeof(RREP));
				sendPacket(&snd,(void *)sendBuffer,sizeof(RREP));
			}
		}
	}
	return;
}

/* this function processes the incoming RERR messages */
void		aodv::processRERR(aodvData *data)
{
	char		*buf;
        u_int32_t	*precAddr;
	sendData	snd;
	bool	result = false;

	u_int64_t	currtime = getcurrtime();

#ifdef DEBUG
   	cout << "adov: processRERR: entered" << endl;
#endif

	precAddr = (u_int32_t *)malloc(sizeof(u_int32_t));

	/* recvBuffer is in htonl() format */
	buf = (char *)recvBuffer;

	if(duringReboot)
	{
#ifdef DEBUG
    	cout << "adov: processRERR: during reboot" << endl;
#endif
		/* create route entries for the neighbor from which the rerr is received*/
		RERR	rerrOb;
		rerrOb.createRERR(buf);

		rtable.rebootNeighborUpdate(data->src_ip,0,(currtime + ALLOWED_HELLO_LOSS*HELLO_INTERVAL));


		/* update both valid and invalid entries in the routing table based 
		 * on the information received in the RERR message */
		rerrOb.updateInvalidEntries();
		rerrOb.updateValidEntries(data->src_ip);
		return;
	}
	else
	{

#ifdef DEBUG
    	cout << "adov: processRERR: not during reboot" << endl;
#endif
		RERR	rerrOb, rerrNewOb;

		/* create a RERR message from the received message */
		rerrOb.createRERR(buf);


		/* incorporate received information into the existing routing table */
		// neighborUpdate();
		rtable.neighborUpdate(data->src_ip,data->src_ip,0,(currtime+ALLOWED_HELLO_LOSS*HELLO_INTERVAL));


		/* only update invalid (infinity metric) route table entries, valid route 
		 * table entries get updated in createNewRerr() */
	
		rerrOb.updateInvalidEntries();

		/* handle rerr with N bit set */
		int n_val = rerrOb.getN();
		if(n_val == 1)
		{
#ifdef DEBUG
    		cout << "adov: processRERR: received RERR with N bit set" << endl;
#endif
			
			rtable_entry	*rtEntry;

			/* dest cnt will always be 1 in this case */
			if(rerrOb.getDestCnt() == 1)
			{
				rtEntry = rtable.findActiveRoute(rerrOb.getUnrchDest());
				if((rtEntry != NULL) && (rtEntry->getNextHop() == data->src_ip))
				{
					/* rerr arrived along the next hop for this dest */
					/* check for precursor list,
					 * and forward the RERR if any precursor exist */

					rtEntry->setDestSeqNum(rerrOb.getUnrchDestSeqNum());

					if(rtEntry->getPrecCnt() > 0)
						generateRERR(rerrOb.getUnrchDest(),1,false);
					else
					{
						/* rerr has reached the originator node */
						cout << "aodv:processRERR: rerr has reached the originator node " << endl;
					}
				}
			}
			return;
		}	

		/* generate a new RERR if needed and send it out */
		
#ifdef DEBUG
  		cout << "adov: processRERR: generate a new RERR" << endl;
#endif
		result = rerrNewOb.createNewRERR(&rerrOb, data->src_ip, precAddr);

		if( result )
		{
#ifdef DEBUG
  			cout << "adov: processRERR: transmit the new RERR" << endl;
#endif
			char	*newBuf;
			int			len;

			len = 4*(1+(2*rerrNewOb.getDestCnt()));
			newBuf = (char *)malloc(sizeof(len));

			memset(newBuf,0,len);

			packBuf(newBuf,&rerrNewOb);

			if(precAddr != NULL)
			{
#ifdef DEBUG
  				cout << "aodv: processRERR: unicast new RERR" << endl;
#endif
				/* unicast RERR to the addr */
				genSndData(&snd,*precAddr,1);
				copyToSendBuf((void *)newBuf, len);
				sendPacket(&snd,(void *)sendBuffer,len);
			}
			else
			{
				/* broadcast RERR */
#ifdef DEBUG
  				cout << "aodv: processRERR: broadcast new RERR" << endl;
#endif
				genSndData(&snd,BROADCAST_ADDR,1);
				copyToSendBuf((void *)newBuf, len);
				sendPacket(&snd,(void *)sendBuffer,len);
			}
			free(precAddr);
			free(newBuf);
		}
		else
			cout << "aodv: processRERR: do not transmit the new RERR " << endl;
	}
	return;
}

/* process received RREP-ACK message */
void		aodv::processRREP_ACK(aodvData *data)
{
	/* remove the rrep_ack timer if set*/

#ifdef DEBUG
	cout << "aodv::processRREP_ACK: entered " << endl;
#endif

	timer_Q.removeRREP_ACK_Timer(data->src_ip);
	return;
}

u_int32_t	aodv::get_next_hop(u_int32_t dst)
{
	/* Here also update the lifetime field */
	rtable_entry	*rtEntry;
	rtEntry = rtable.findActiveRoute(dst);
	if(rtEntry != NULL)
	{
		rtEntry->setLifeTime(MAX(rtEntry->getLifeTime(),(getcurrtime()+ACTIVE_ROUTE_TIMEOUT)));
		return rtEntry->getNextHop();
	}
	else
		return 0;

}

/* send and recv message from Local Agent, now contain includes ODS service*/
void	recv_la_data_handler(void){
	cout<<"process in [recv_la_data()]\n"<<endl;
	aodvOb.recv_la_data();
}

void aodv::recv_la_data(){
	int result;
	struct rt_info	*rInfo;

	rInfo = (struct rt_info *)malloc(sizeof(struct rt_info));
	
	char recvbuf[1024];
	memset(recvbuf, 0, 1024);
	result = recv(la_sock, recvbuf, 1024, 0);

	LAMessage now_message;
	now_message.parse_stream(recvbuf);

	rInfo->dst_ip = now_message.dst_address;
	rInfo->src_ip = now_message.src_address;
	rInfo->protocol = now_message.protocol;

	if(result <=0 )
	{
		cout << "read_route_request(): returned error code " << result << endl;

	}
	else if (result == 0)
		cout << "Duplicate route_request_read: Since route discovery already in progress, do nothing " << endl;
	else if ( result > 0)
	{
#ifdef DEBUG
		cout << " recv_asl_data: got data packet from asl" << endl;
#endif
		process_asl(rInfo);
	}

	free(rInfo);
	return;
}


/* handler function for data received on asl socket, is called whenever a packet arrives on
 * asl socket*/
void 	recv_asl_data_handler(void)
{
#ifdef DEBUG
	cout << "recv_asl_data_handler: got data" << endl;
#endif
	aodvOb.recv_asl_data();
}

void	aodv::recv_asl_data(){
#ifdef ASL
	int result;
	struct route_info	*rInfo;

	rInfo = (struct route_info *)malloc(sizeof(struct route_info));
	
#ifdef DEBUG
	cout << "recv_asl_data Entered" << endl;
#endif

	result = read_route_request(asl_sock, rInfo);
	if(result <=0 )
	{
		cout << "read_route_request(): returned error code " << result << endl;

	}
	else if (result == 0)
		cout << "Duplicate route_request_read: Since route discovery already in progress, do nothing " << endl;
	else if ( result > 0)
	{
#ifdef DEBUG
		cout << " recv_asl_data: got data packet from asl" << endl;
#endif
		process_asl(rInfo);
	}

	free(rInfo);
	return;
#endif
}

void	aodv::process_asl(struct rt_info *rInfo){
	u_int8_t		rflags, protocol;
	u_int32_t		src_ip, dst_ip;
	RERR	rerrOb;

	dst_ip = (rInfo->dst_ip);
	src_ip = (rInfo->src_ip);
	protocol = (rInfo->protocol);

	cout << "aodv: process_la: entered, dst is " << getDotIP(dst_ip) << endl;
	cout << "aodv: process_la: entered, src is " << getDotIP(src_ip) << endl;
#ifdef DEBUG
	cout << "aodv: process_asl: entered, dst is " << getDotIP(dst_ip) << endl;
	cout << "aodv: process_asl: entered, src is " << getDotIP(src_ip) << endl;
#endif

	/* start route discovery for received dest IP */

	if(duringReboot)
	{
		/* generate a rerr for this destination */
		/* reset the reboot timer */
		
		

// #ifdef DEBUG
			cout << "process_asl: during reboot" << endl;
// #endif
		if(src_ip == g_my_ip)
		{
			/* locally generated data packets can not be forwarded during reboot */
			LAMessage feedback_message;
			feedback_message.cmd = LA_ODS_REPLY;
			feedback_message.dst_address = dst_ip;
			feedback_message.src_address = 0;
			feedback_message.protocol = 0;
			feedback_message.result = IP_NOFOUND;
			
			char send_buf[1024];
			int len = 0;
			len = feedback_message.generate_stream(send_buf, 1024);
			int send_ret = send(la_sock, send_buf, len, 0);
			// route_discovery_done(dst_ip, ASL_NO_ROUTE);
			return;
		}

		if(src_ip != g_my_ip)
		{
// #ifdef DEBUG
			cout << "process_asl: during reboot, got non local packet " << endl;
// #endif
			timer_Q.resetRebootTimer();
			LAMessage feedback_message;
			feedback_message.cmd = LA_ODS_REPLY;
			feedback_message.dst_address = dst_ip;
			feedback_message.src_address = 0;
			feedback_message.protocol = 0;
			feedback_message.result = IP_NOFOUND;
			
			char send_buf[1024];
			int len = 0;
			len = feedback_message.generate_stream(send_buf, 1024);
			int send_ret = send(la_sock, send_buf, len, 0);
			// route_discovery_done(dst_ip, ASL_NO_ROUTE);

			generateRERR(dst_ip, 0, true);
		}
		return;
	}

// #ifdef DEBUG
		cout << "aodv: process_asl: not during reboot " << endl;
// #endif
		
        /* check if a valid route table entry exist for this destination.
	 * it might have happened that this entry is not yet added to the
	 * kernel routing table because of ioctl latency */
	if(rtable.findActiveRoute(dst_ip))
	{
// #ifdef DEBUG
		cout << "aodv: process_asl: route already exist " << endl;
// #endif
		LAMessage feedback_message;
		feedback_message.cmd = LA_ODS_REPLY;
		feedback_message.dst_address = dst_ip;
		feedback_message.src_address = 0;
		feedback_message.protocol = 0;
		feedback_message.result = IP_FOUND;
		
		char send_buf[1024];
		int len = 0;
		len = feedback_message.generate_stream(send_buf, 1024);
		int send_ret = send(la_sock, send_buf, len, 0);
		// route_discovery_done(dst_ip, ASL_ROUTE_FOUND);
		return;
	}

	if(src_ip == g_my_ip)
	{
// #ifdef DEBUG
		cout << "aodv: process_asl: locally generated data packet" << endl;
		cout << "aodv: process_asl: dest ip is " << getDotIP(dst_ip) << endl;
// #endif
		/* data packet is generated locally */
		if(protocol == IPPROTO_TCP)
		{
			/* set the G flag */
// #ifdef DEBUG
			cout << "aodv: process_asl:set G flag "  << endl;
// #endif
			rflags = 1;
		}
		/* send out a rreq for locally generated data packets */
		sendRREQ(dst_ip, rflags);
	}
	else
	{
		/* in case of intermediate node , check if local repair can be done */
		/* the way ASL is designed, if a data packet is received for a dest
		 * and if that dest happens to be in local repair list, then no 
		 * earlier attempt for local repair has been made, so the broken route
		 * check for local repair is not really needed, but I anyway do it
		 * just in case */

		if(local_repair.findInList(dst_ip) != (list<local_repair_entry>::iterator)NULL)
		{
// #ifdef DEBUG
			cout << "aodv: process_asl:dest in local repair list "  << endl;
// #endif
			/* do local repair here */
			if(protocol == IPPROTO_TCP)
				rflags = 1;
			else
				rflags = 0;

			rtable_entry	*rtEntry;
			rtEntry = rtable.findRoute(dst_ip);
			if((rtEntry != NULL) && (rtEntry->getRFlags()== BROKEN_ROUTE))
			{
				cout << "aodv:process_asl: do loca repair " << endl;
				doLocalRepair(dst_ip, src_ip, rflags);
			}
			else
			{
				cout << "aodv: process_asl: local repair in progress.. " << endl;
				cout << "do not return any result to ASL now " << endl;
				return;
			}
		}
		else
		{
			/* return "route can not be discovered to asl 
			 * and generate a RERR */
			LAMessage feedback_message;
			feedback_message.cmd = LA_ODS_REPLY;
			feedback_message.dst_address = dst_ip;
			feedback_message.src_address = 0;
			feedback_message.protocol = 0;
			feedback_message.result = IP_NOFOUND;
			
			char send_buf[1024];
			int len = 0;
			len = feedback_message.generate_stream(send_buf, 1024);
			int send_ret = send(la_sock, send_buf, len, 0);
			// route_discovery_done(dst_ip, ASL_NO_ROUTE);

			rtable_entry	*rtEntry;

			cout << "aodv:process_asl: no route exist, return NO_ROUTE " << endl; 
			rtEntry = rtable.findRoute(dst_ip);
			if(rtEntry != NULL)
				generateRERR(dst_ip,0,false);
			else
				generateRERR(dst_ip,0,true);

		}
	}
	return;
}

/* this function is called when data is received from asl */
void 	aodv::process_asl(struct route_info *rInfo)
{
#ifdef ASL
	u_int8_t		rflags, protocol;
	u_int32_t		src_ip, dst_ip;
	RERR	rerrOb;

	dst_ip = (rInfo->dest_ip);
	src_ip = (rInfo->src_ip);
	protocol = (rInfo->protocol);

#ifdef DEBUG
	cout << "aodv: process_asl: entered, dst is " << getDotIP(dst_ip) << endl;
	cout << "aodv: process_asl: entered, src is " << getDotIP(src_ip) << endl;
#endif

	/* start route discovery for received dest IP */

	if(duringReboot)
	{
		/* generate a rerr for this destination */
		/* reset the reboot timer */
		
		

#ifdef DEBUG
			cout << "process_asl: during reboot" << endl;
#endif
		if(src_ip == g_my_ip)
		{
			/* locally generated data packets can not be forwarded during reboot */
			route_discovery_done(dst_ip, ASL_NO_ROUTE);
			return;
		}

		if(src_ip != g_my_ip)
		{
#ifdef DEBUG
			cout << "process_asl: during reboot, got non local packet " << endl;
#endif
			timer_Q.resetRebootTimer();
			route_discovery_done(dst_ip, ASL_NO_ROUTE);

			generateRERR(dst_ip, 0, true);
		}
		return;
	}

#ifdef DEBUG
		cout << "aodv: process_asl: not during reboot " << endl;
#endif
		
        /* check if a valid route table entry exist for this destination.
	 * it might have happened that this entry is not yet added to the
	 * kernel routing table because of ioctl latency */
	if(rtable.findActiveRoute(dst_ip))
	{
#ifdef DEBUG
		cout << "aodv: process_asl: route already exist " << endl;
#endif
		route_discovery_done(dst_ip, ASL_ROUTE_FOUND);
		return;
	}

	if(src_ip == g_my_ip)
	{
#ifdef DEBUG
		cout << "aodv: process_asl: locally generated data packet" << endl;
		cout << "aodv: process_asl: dest ip is " << getDotIP(dst_ip) << endl;
#endif
		/* data packet is generated locally */
		if(protocol == IPPROTO_TCP)
		{
			/* set the G flag */
#ifdef DEBUG
			cout << "aodv: process_asl:set G flag "  << endl;
#endif
			rflags = 1;
		}
		/* send out a rreq for locally generated data packets */
		sendRREQ(dst_ip, rflags);
	}
	else
	{
		/* in case of intermediate node , check if local repair can be done */
		/* the way ASL is designed, if a data packet is received for a dest
		 * and if that dest happens to be in local repair list, then no 
		 * earlier attempt for local repair has been made, so the broken route
		 * check for local repair is not really needed, but I anyway do it
		 * just in case */

		if(local_repair.findInList(dst_ip) != (list<local_repair_entry>::iterator)NULL)
		{
#ifdef DEBUG
			cout << "aodv: process_asl:dest in local repair list "  << endl;
#endif
			/* do local repair here */
			if(protocol == IPPROTO_TCP)
				rflags = 1;
			else
				rflags = 0;

			rtable_entry	*rtEntry;
			rtEntry = rtable.findRoute(dst_ip);
			if((rtEntry != NULL) && (rtEntry->getRFlags()== BROKEN_ROUTE))
			{
				cout << "aodv:process_asl: do loca repair " << endl;
				doLocalRepair(dst_ip, src_ip, rflags);
			}
			else
			{
				cout << "aodv: process_asl: local repair in progress.. " << endl;
				cout << "do not return any result to ASL now " << endl;
				return;
			}
		}
		else
		{
			/* return "route can not be discovered to asl 
			 * and generate a RERR */
			route_discovery_done(dst_ip, ASL_NO_ROUTE);

			rtable_entry	*rtEntry;

			cout << "aodv:process_asl: no route exist, return NO_ROUTE " << endl; 
			rtEntry = rtable.findRoute(dst_ip);
			if(rtEntry != NULL)
				generateRERR(dst_ip,0,false);
			else
				generateRERR(dst_ip,0,true);

		}
	}
	 return;
	#endif
}

void	aodv::packBuf(char	*buf, RERR *rerrOb)
{
	cout << "aodv: packBuf: entered" << endl;
	rerrOb->copyIntoBuf(buf);
	/* after this buf will have packed rerr in htonl() format */
}

/* this function is called to carry local repair */
int	aodv::doLocalRepair(u_int32_t dst, u_int32_t src, u_int8_t rflags)
{

	RREQ	rreqOb;
	u_int32_t	orgHopCnt=0;
	int			MIN_REPAIR_TTL, ttl;
	sendData	snd;
	rtable_entry	*rtEntry, *rtDestEntry;

#ifdef DEBUG
	cout << " aodv: doLocalRepair: entered" << endl;
#endif

	rreqOb.createRREQ(dst,rflags);

	rtEntry = rtable.findRoute(src);
	if(rtEntry != NULL )
	{
		cout << "doLocalRepair: getting org hop cnt " << endl;
		if(rtEntry->getHopCnt() == INFINITY)
			orgHopCnt = rtEntry->getLastHopCnt();
		else
			orgHopCnt = rtEntry->getHopCnt();
	}

	rtDestEntry = rtable.findRoute(dst);
	if(rtDestEntry != NULL)
	{
		//rtDestEntry->setRFlags(LOCAL_REPAIR_ROUTE);
		MIN_REPAIR_TTL = rtDestEntry->getLastHopCnt();
	}
	else
		MIN_REPAIR_TTL = 0;

	//ttl = (int)MAX(MIN_REPAIR_TTL, (0.5*orgHopCnt)) + LOCAL_ADD_TTL;
	ttl = (MIN_REPAIR_TTL) + LOCAL_ADD_TTL;

#ifdef DEBUG
	cout << " aodv: doLocalRepair: orgHopCnt is " << orgHopCnt << "ttl is " << ttl << endl;
#endif

	setLRTimer(dst, ttl);

	genSndData(&snd,BROADCAST_ADDR, ttl);
	copyToSendBuf((void *)&rreqOb, sizeof(RREQ));
	sendPacket(&snd,(void*)sendBuffer,sizeof(RREQ));

	return 0;
}

int	aodv::sendRREQ(u_int32_t dst_ip, u_int8_t flags)
{

	int	retries=0;
	sendData	snd;

#ifdef DEBUG
	cout << "sendRREQ: entered" << endl;
#endif

	/* this should never be true, checking to be on the safer side */
	if(dst_ip == g_my_ip)
	{
		/* do not send a rreq for itself */
		return 0;
	}

	RREQ	rreqOb;
	rreqOb.createRREQ(dst_ip,flags);
	int		ttl;

	/* set the lifetime of route entry */
	/* a route table entry waiting for a RREP
	 * should not be expunged before currtime+PATH+TRAVERSAL_TIME(sec 6.4) */
	rtable_entry 	*rtEntry;
	rtEntry = rtable.findRoute(dst_ip);

	if(rtEntry != NULL)
		rtEntry->setLifeTime(getcurrtime()+PATH_TRAVERSAL_TIME);


	/* do expanding ring search */
	if(expandingRingSearch)
	{
#ifdef DEBUG
		cout << "aodv: sendRREQ: expanding ring search " << endl;
#endif
		ttl=(int)rtable.getTTL(dst_ip);
		if(ttl == 0)
			ttl=TTL_START;
		else
		{
			/* an entry for the destination already exists */
			/* update the lifetime of this entry */
			ttl=ttl+TTL_INCREMENT;
		}
	}

#ifdef DEBUG
	cout << "aodv: sendRREQ: dest ip is " << getDotIP(rreqOb.getDestIP()) << endl;
#endif
	rreqL.addToList(rreqOb, retries, ttl );

	setRREQTimer(dst_ip, ttl);

	genSndData(&snd,BROADCAST_ADDR, ttl);
	copyToSendBuf((void *)&rreqOb, sizeof(RREQ));
	sendPacket(&snd,(void*)sendBuffer,sizeof(RREQ));

	return 0;
}

/* this function sets the RREQ timer */
int	aodv::setRREQTimer(u_int32_t dst, int ttl)
{

	struct timerData	tData;
	tData.type = RREQ_TIMER;
	tData.data = dst;

	if(expandingRingSearch)
	{
		timer_Q.set_timer(2*ttl*NODE_TRAVERSAL_TIME, resendRREQ_handler, (void *)&tData);
	}	
	else
		timer_Q.set_timer(NET_TRAVERSAL_TIME, resendRREQ_handler, (void *)&tData);

	return 0;
}


/* this function sets the local repiar timer */
int	aodv::setLRTimer(u_int32_t dst, int ttl)
{

	struct timerData	tData;
	tData.type = LR_TIMER;
	tData.data = dst;

#ifdef DEBUG
	cout << "aodv: setLRTimer: entered" << endl;
#endif

	timer_Q.set_timer(2*ttl*NODE_TRAVERSAL_TIME, LR_timer_handler, (void *)&tData);
	return 0;
}

void	LR_timer_handler(void *data)
{
#ifdef DEBUG
	cout << "aodv: LR_timer_handler: entered" << endl;
#endif

	aodvOb.LR_handler(data);
	return;
}

void	aodv::LR_handler(void *data)
{
	u_int32_t	dst;
	RERR		rerrOb;
	rtable_entry	*rtEntry;

	struct timerData	*tData;

	tData = (struct timerData*)data;
	dst = tData->data;

	/* return route status to ASL */
	LAMessage feedback_message;
	feedback_message.cmd = LA_ODS_REPLY;
	feedback_message.dst_address = dst;
	feedback_message.src_address = 0;
	feedback_message.protocol = 0;
	feedback_message.result = IP_NOFOUND;
	
	char send_buf[1024];
	int len = 0;
	len = feedback_message.generate_stream(send_buf, 1024);
	int send_ret = send(la_sock, send_buf, len, 0);
	//zyp
	// if( route_discovery_done(dst, ASL_NO_ROUTE) < 0)
	// 	cout << " Error: LR_handler: route_discovery_done() " << endl;

	/* generate and send a rerr if no RREP was received for local repair attempt*/


	rtEntry = rtable.findRoute(dst);
	if(rtEntry != NULL)
	{
		rtEntry->setLastHopCnt(rtEntry->getHopCnt());
		rtEntry->setHopCnt(INFINITY);
		rtEntry->setLifeTime(getcurrtime()+DELETE_PERIOD);
		rtEntry->setRFlags(INVALID_ROUTE);
	}

	/* generate a RERR for this dest */
	generateRERR(dst,0,false);

	/* also remove the packet from the local repair list */
	list<local_repair_entry>::iterator	iter;

	iter = local_repair.findInList(dst);
	if( iter != (std::list<local_repair_entry>::iterator)NULL)
	{
		local_repair.deleteFromList(iter);
	}
	
	return;
}

/* handler function for RREQ timer */
void	resendRREQ_handler(void *val)
{
	aodvOb.resendRREQ(val);
}

void	aodv::resendRREQ(void *val)
{
	struct timerData	*tData;
	rreq_list_entry	 *rreqEntry;

	tData = (struct timerData*)val;
	u_int32_t	dst = tData->data;

#ifdef DEBUG
	cout << "aodv: resendRREQ: dest ip is " << getDotIP(dst) << endl;
#endif

	rreqEntry = rreqL.inList(dst);
	if(rreqEntry != NULL)
	{
		sendData	snd;

		cout << "resendRREQ: entry in rreqL " << endl;

		if(expandingRingSearch)
		{
			cout << "resendRREQ: expandingRingSearch " << endl;
			if(rreqEntry->getTTL() == NET_DIAMETER)
			{	
				rreqEntry->setRetries(rreqEntry->getRetries()+1);
			}
			else
			{
				rreqEntry->setTTL((rreqEntry->getTTL())+TTL_INCREMENT);
				if(rreqEntry->getTTL() >= TTL_THRESHOLD)
				{
					rreqEntry->setTTL(NET_DIAMETER);
					rreqEntry->setRetries(rreqEntry->getRetries()+1);
				}
			}
		}
		else
		{
			cout << "resendRREQ: no expandingRingSearch " << endl;
			rreqEntry->setRetries(rreqEntry->getRetries()+1);
		}

		cout << "resendRREQ: retries value is " << rreqEntry->getRetries() << endl;

		if(rreqEntry->getRetries() > RREQ_RETRIES)
		{
#ifdef DEBUG
			cout << "resendRREQ: max retries done, not resending RREQ" << endl;
#endif
			/* delete this entry from the rreq list */
			/* also return a no route found to asl */
			rreqL.deleteFromList(dst);
			LAMessage feedback_message;
			feedback_message.cmd = LA_ODS_REPLY;
			feedback_message.dst_address = dst;
			feedback_message.src_address = 0;
			feedback_message.protocol = 0;
			feedback_message.result = IP_NOFOUND;
			
			char send_buf[1024];
			int len = 0;
			len = feedback_message.generate_stream(send_buf, 1024);
			int send_ret = send(la_sock, send_buf, len, 0);
			//zyp
			// if( route_discovery_done(dst, ASL_NO_ROUTE) < 0)
			// 	cout << " Error: route_discovery_done() " << endl;
			return ;
		}
	
		rreqEntry->setRreqId(++rreqId);
		rreqEntry->setDestSeqNum(rtable.getDestSeqNum(dst));
		rreqEntry->setSrcSeqNum(++localSeqNum);

		cout << "resendRREQ: ttl value is " << rreqEntry->getTTL() << endl;

		/* also update the rreq list timer */
		setRREQTimer(dst, rreqEntry->getTTL());

		genSndData(&snd,BROADCAST_ADDR, rreqEntry->getTTL());

		cout << "resendRREQ: ttl value is " << snd.getTTL() << endl;

		copyToSendBuf((void *)(rreqEntry->getRreqOb()), sizeof(RREQ));
		sendPacket(&snd,(void *)sendBuffer,sizeof(RREQ));
	}
	else
	{
		cout << "Error resendRREQ: entry not in rreq List " << endl;
	}
	return ;
}

void	aodv::copyToSendBuf(void *data, int len)
{
	memset(sendBuffer,0,AODVBUFSIZE);
	memcpy(sendBuffer,data,len);
	return;
}

/* generic function to transmit and outgoing packet */
void	aodv::sendPacket(sendData *snd, void *data, int data_len)
{
	struct sockaddr_in	dst;
	int		len, ttl;

	ttl = snd->getTTL();

	cout << "sendPacket: ttl is " << snd->getTTL() << endl;
	cout << "sendPacket: ttl is " << ttl << endl;

	if(ttl == 0)
		return;

#ifdef DEBUG
	cout << "sendPacket: entered" << endl;
#endif

	if(snd->getDestIP() == BROADCAST_ADDR)
		needToSendHello = false;

	bzero((void *)&dst, sizeof(dst));
  	dst.sin_family = AF_INET;
  	dst.sin_port = htons(AODV_PORT);     
    dst.sin_addr.s_addr = snd->getDestIP();
	
	if (ttl > 0 && ttl < 256){ 
		cout << "sendPacket: setting ttl " << endl;

    	if (setsockopt(aodvSock.getSock(), SOL_IP, IP_TTL, &(ttl), sizeof(ttl)) < 0)
		{
			perror("Error setting TTL");
		}
		else
			cout << " sendPacket: ttl set to " << ttl << endl;
	}

	if ((len = sendto(aodvSock.getSock(), data, data_len, 0, 
	     (struct sockaddr *)&dst, sizeof(struct sockaddr))) < 0) {
			perror("Error in sendto");
  	} 

#ifdef DEBUG
	cout << "sendPacket: exiting" << endl;
#endif

	return;
}	


int	aodv::aodv_daemon(char *interface)
{
	/* Initialization */

#ifdef DEBUG
	cout << "inside aodv_daemon" << endl;
#endif

	registerSignal();	//注册一些信号量

	/* start all relevant timers here */
	/* set the reboot timer */
	setRebootTimer();

	/* set periodic refresh timer to periodically update route table entries */
	setPeriodicTimer();

	/* intialization */
	aodv_init(interface);

	/* start the main daemon loop */
	mainDaemonLoop();
	return 0;
}

void	aodv::setRebootTimer()
{
	struct timerData	tData;
	tData.type = REBOOT_TIMER;
	tData.data = 0;
	
	timer_Q.set_timer_first(DELETE_PERIOD, rebootHandler_stub, (void *)&tData);
	return;
}
//功能:1.把所有有效的路由条目添加到内核中路由表中    2.发送hello消息（每隔一段时间就会发送）
void	rebootHandler_stub(void *data)
{
	aodvOb.rebootHandler();
}

void	aodv::setPeriodicTimer()
{
	struct timerData	tData;
	tData.type = PERIODIC_TIMER;
	tData.data = 0;
	
	timer_Q.set_timer(PERIODIC_INTERVAL, periodicHandler_stub, (void *)&tData);
	return;
}

void	periodicHandler_stub(void *data)
{
	aodvOb.periodicHandler();
}
//周期定时器到时之后，又会回到这里
void 	aodv::periodicHandler()
{
#ifdef DEBUG
	cout << "periodicHandler: add a periodic timer " << endl;
#endif
	/* insert new periodic timer entry */
	struct	timerData	tData;
	tData.type = PERIODIC_TIMER;
	tData.data = 0;
	timer_Q.timer_add(PERIODIC_INTERVAL, periodicHandler_stub, (void *)&tData);

	/* this timer acts as periodic timer for refreshing route table entries */
	rtable.refreshEntries();
	fw_rreqL.updateEntries();
	black_list.updateEntries();

	return;
}
//reboot定时器到时之后
void	aodv::rebootHandler()
{
#ifdef DEBUG
	cout << "rebootHandler: entered" << endl;
#endif

	duringReboot = false;
	
	/* enter all existing valid route entries into kernel route table */
	rtable.addAllValidRoutes();

	/* start the hello message */
	if(helloSendFlag)
		aodvOb.sendHello();
}
//注册信号量：定时器、中止
int	aodv::registerSignal()
{
	signal(SIGALRM,global_handler);//定时器到时
	signal(SIGINT,sigint_timer_handler);	// ctrl+c 中止信号
	return 0;
}

void	global_handler(int type)	//-OK
{
	aodvOb.handler(type);	//-OK
}

void 	sigint_timer_handler(int type)
{
	aodvOb.sigint_handler(type);
}

/* this function sends out a hello message */  //发送一个hello消息，而且每隔一段时间都发一遍
void	aodv::sendHello()
{
	/* create and send a hello message */
	sendData	snd;

#ifdef DEBUG
	cout << "sendHello: entered" << endl;
#endif

	if(needToSendHello)
	{
#ifdef DEBUG
		cout << "sendHello: send a hello message" << endl;
#endif

		RREP	rrepOb(g_my_ip,localSeqNum, 0, 0, (ALLOWED_HELLO_LOSS*HELLO_INTERVAL));
		genSndData(&snd, BROADCAST_ADDR, 1);
		copyToSendBuf((void *)&rrepOb, sizeof(RREP));
		sendPacket(&snd,(void *)sendBuffer,sizeof(RREP));
	}

#ifdef DEBUG
	cout << "sendHello: add a hello timer " << endl;
#endif
	struct	timerData	tData;
	tData.type = HELLO_TIMER;
	tData.data = 0;
	timer_Q.timer_add(HELLO_INTERVAL, sendHello_handler, (void *)&tData);

	needToSendHello = true;

#ifdef DEBUG
	cout << "sendHello: exiting " << endl;
#endif
	return;

}

void	sendHello_handler( void * data)
{
	aodvOb.sendHello();
}
void	aodv::handler(int type)	//-OK
{
	timer_Q.scheduleTimer();	//-OK
}

void	aodv::sigint_handler(int type)
{
	// close_route_request(asl_sock);
	free(recvBuffer);
	free(sendBuffer);
	exit(1);
}

void	aodv::generateRERR(u_int32_t dst, u_int8_t N_flag, bool brdFlag)
{
	RERR	rerrOb;
	sendData	snd;
	rtable_entry	*rtEntry;
	u_int32_t	precCnt=0, seqNum=0;

	rtEntry = rtable.findRoute(dst);

#ifdef DEBUG
	cout << "aodv: generateRERR: entered" << endl;
#endif

	cout << "aodv::generateRERR: rerr type is " << (int)rerrOb.getType() << endl;

	if(rtEntry != NULL)
	{
		seqNum = rtEntry->getDestSeqNum();
		precCnt = rtEntry->getPrecCnt();
	}
	

	if(precCnt > 0 || brdFlag)
	{

		cout << "aodv: generateRERR: send rerr " << endl;

		rerrOb.setNFlag(N_flag);
		rerrOb.setDestCnt(1);
		rerrOb.setUnrchDest((dst),seqNum);

		cout << "aodv::generateRERR: rerr type is " << (int)rerrOb.getType() << endl;

		/* pack rerr for sending */
		char	*buf;
		int			len;

		len = 4*(1+(2*rerrOb.getDestCnt()));
		buf = (char *)malloc(sizeof(len));

		memset(buf,0,len);

#ifdef DEBUG
		cout << " aodv:generateRERR: send rerr type is " << (int)rerrOb.getType() << endl;
#endif
		packBuf(buf,&rerrOb);

		if(precCnt == 1)
		{
			/* unicast RERR */
			u_int32_t	precAddr;

			cout << "aodv: generateRERR: unicast rerr " << endl;
			rtEntry->getPrecAddr(&precAddr);
			aodvOb.genSndData(&snd,precAddr,1);
			aodvOb.copyToSendBuf((void *)buf, len);
			aodvOb.sendPacket(&snd,(void *)sendBuffer,len);

		}	
		else
		{
			/* broadcast RERR */

			cout << "aodv: generateRERR: broadcast rerr " << endl;

			aodvOb.genSndData(&snd,BROADCAST_ADDR,1);
			aodvOb.copyToSendBuf((void *)buf, len);
			aodvOb.sendPacket(&snd,(void *)sendBuffer,len);
		}
		free(buf);
	}
	return;
}

/* the main rouitng daemon loop function */
void	aodv::mainDaemonLoop()
{
	int nfds=0, ret;
	fd_set	rbits, rfds;

#ifdef DEBUG
	cout << "mainDaemonLoop Entered " << endl;
#endif

	FD_ZERO(&rbits);

	for(int i=0; i< handler_cnt; i++)
	{
		FD_SET(handlers[i].fd,&rbits);
		if(handlers[i].fd >= nfds)
			nfds = handlers[i].fd +1;
	}

	while(1)
	{
		memcpy((char *)&rfds, (char *)&rbits, sizeof(rbits));

		if( (ret = select(nfds, &rfds, NULL, NULL, NULL)) < 0)
		{
			cout << "原来的Error in select"<< endl;
			//fprintf(stderr, "\n");
    		//exit(EXIT_FAILURE);
		}
		else
		{
			for(int i=0;i<handler_cnt;i++)
			{
				if(FD_ISSET(handlers[i].fd, &rfds))
				{
					(*handlers[i].func)();
				}
			}
		}
	}
	
	return;
}
