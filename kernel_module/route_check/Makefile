obj-m += route_check.o

route_check-objs := module_main.o    route_cache.o  utils.o

KERNELBUILD := /lib/modules/$(shell uname -r)/build
CURRENT_PATH := $(shell pwd)


all:
	make -C $(KERNELBUILD) M=$(CURRENT_PATH) modules  

clean:
	make -C $(KERNELBUILD) M=$(CURRENT_PATH) clean  

install:
	@echo Installing route_check.o in /lib/modules/$(KVERSION)/kernel/net/ipv4/
	@cp -f ./route_check.o /lib/modules/$(KVERSION)/kernel/net/ipv4/