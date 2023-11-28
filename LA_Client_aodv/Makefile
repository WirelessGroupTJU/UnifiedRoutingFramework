#Makefile for aodv daemon


#CFLAGS	= -DDEBUG -Wall -g -Wcast-align  
CFLAGS	= -w -g -Wcast-align  -fcompare-debug-second 
LIBS	= -lfo -lcommon
CC	=g++ $(CFLAGS)
# CC	=mipsel-openwrt-linux-gcc $(CFLAGS)

OBJS	= main.o aodv.o aodvSocket.o localRepair.o rerr.o routingTable.o rrep.o rreq.o \
		rreqPendingList.o rtable_entry.o timer.o timerQueue.o utils.o fwRreqList.o blacklist.o

all : aodvd

.cc.o: 
	$(CC) -c $<

aodvd 	: $(OBJS)
		  $(CC) -o aodvd $(OBJS) $(LIBS)

clean		:
		  rm -f *.o 
		  rm -f aodvd

#broadcast.o : main.h broadcast.h macros.h
#main.o : main.h broadcast.h macros.h
#routeTable.o : main.h broadcast.h macros.h routeTable.h
#rtable.o : main.h broadcast.h macros.h rtable.h
#timer.o : main.h broadcast.h macros.h timer.h

