######################################
#
#
######################################
  
#target you can change test to what you want
#共享库文件名，lib*.so
TARGET  := libods.so
  
#compile and lib parameter
#编译参数
CC      := g++
LIBS    :=
LDFLAGS :=
DEFINES :=
INCLUDE := 
CFLAGS  := 
CXXFLAGS:= $(CFLAGS)
SHARE   := -fPIC -shared -o
  
#i think you should do anything here
#下面的基本上不需要做任何改动了
  
#source file
#源文件，自动找所有.c和.cpp文件，并将目标定义为同名.o文件
SOURCE  := $(wildcard *.c) $(wildcard *.cpp)
OBJS    := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))
  
.PHONY : everything objs clean veryclean rebuild install uninstall
  
everything : $(TARGET)
  
all : $(TARGET)
  
objs : $(OBJS)
  
rebuild: veryclean everything
                
clean :
	rm -fr *.o
	rm *.so
	cd ODS_kernel && $(MAKE) clean
    
veryclean : clean
	rm -fr $(TARGET)

install:
	cp $(TARGET) /usr/lib
	ldconfig
	cd ODS_kernel && insmod ods_kernel_module.ko

uninstall:
	rmmod ods_kernel_module



$(OBJS)	:
	$(CC) -fPIC -c $(SOURCE)

$(TARGET) : $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $(OBJS) $(LDFLAGS) $(LIBS)
	cd ODS_kernel && $(MAKE)