obj-m += ods_kernel_module.o

ods_kernel_module-objs := ODS_kernel_module.o	packet_judgment.o	packet_queue.o	info_transmission.o	utils.o

KERNELBUILD := /lib/modules/$(shell uname -r)/build
CURRENT_PATH := $(shell pwd)


all:
	make -I /usr/src/linux-headers-5.13.0-39-generic/include  -C $(KERNELBUILD) M=$(CURRENT_PATH) modules  

clean:
	make -C $(KERNELBUILD) M=$(CURRENT_PATH) clean