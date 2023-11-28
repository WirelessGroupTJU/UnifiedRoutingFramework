/************************************************************************
			       ---AODV-UIUC---
			       
This software can be used under GNU General Public License.

Author: Binita Gupta <binita@uiuc.edu>
	University of Illinois, Urbana-Champaign
***********************************************************************/



#include "common.h"

using namespace std;

/* Global Variables */
aodv	aodvOb;

u_int32_t	rreqId=0;	/* globally maintained rreq ID */
u_int32_t	localSeqNum=0;  /* sequence number of the local node */
u_int32_t	g_my_ip;	/* IP address of the local node */
int			asl_sock; /* asl socket descriptor */

int			la_sock; /* Local Agent client descriptor */

int			ksock;	/* socket to communicate with the kernel route table */

routingTable 	rtable;		/* routing table object */
rreqPendingList		rreqL;  /* list of rreqs for which no rrep has been received yet */
fwRreqList			fw_rreqL; /* list of rreqs forwarded by the local node */ 
localRepair		local_repair; /* object to implement local repair feature */
timerQueue		timer_Q;      /* queue of timers in the system */
blacklist		black_list;   /* object to store list of nodes in the black list */

char		*recvBuffer;	     /* global buffer to store received message */
char		*sendBuffer;	    /* buffer to generate message to be sent out */
char 		interface[10];

bool  	helloSendFlag=true;
bool 	expandingRingSearch=true;
bool	rrepAckFlag=false;
bool	duringReboot = true;    /* this flag is set if the local node's reboot timer has not expired yet */
bool	needToSendHello = true;	/* this is probably not used in this implementation */
bool	localRepairFlag = true; 

/* end - Global variables */

int		parse_arguments(int argc, char *argv[]);
void	setRebootTimer();
void	setPeriodicTimer();
void	rebootHandler(void *tData);
void	periodicHandler(void *tData);


int main(int argc, char* argv[])
{

if(parse_arguments(argc,argv) == -1)
	exit(1);

#ifdef DEBUG
cout << "Inside main " << endl;
#endif

/* allocate global buffers */
recvBuffer = (char *)malloc(AODVBUFSIZE);
sendBuffer = (char *)malloc(AODVBUFSIZE);

/* call the daemon function with the interface argument */
aodvOb.aodv_daemon(interface);

return 0;
}

/* this funtion parses the command line arguments */
int parse_arguments(int argc, char *argv[])
{
	int hello_flag, ringSearch, rrepAck, lRepair;

	if(argc < 1)
	{
		cout << "Usage: aodv -i <interface name>  -f <hello Flag> -e <expandingRIngSearch> -a <rrepAck>" << endl;
		exit(1);
	}


	for (int i=1;i<argc;i++)
	{
		if(argv[i][0]=='-'){
			switch(argv[i][1]){
				case 'i':
					/* interface name */
					strcpy(interface,argv[++i]);
					break;
				case 'f':
					/* hello message flag, by default hello messages are on */
					hello_flag = atoi(argv[++i]);
					if(hello_flag == 0)
						helloSendFlag = false;
					break;
				case 'e':
					/* enable expanding ring search, by default it is on */
					ringSearch = atoi(argv[++i]);
					if( ringSearch == 0)
						expandingRingSearch = false;
					break;
				case 'a':
					/* setting this flag true sets the A bit on in every RREP, so that the neighbor responds with a RREP-ACK */
					rrepAck = atoi(argv[++i]);
					if( rrepAck == 1)
						rrepAckFlag = true;
					break;
				case 'r':
					/* this emables local repair, by default is is enabled */
					lRepair = atoi(argv[++i]);
					if( lRepair == 0)
						localRepairFlag = false;
					break;
				case 'h':
					cout << "Usage: aodv -i <interface name>  -f <hello Flag> -e <expandingRIngSearch> -a <rrepAck> -r <localRepair>" << endl;
					exit(1);
				default :
					cout << " should never come here " << endl;
					break;
			}
		}
	}
	return 0;
}

