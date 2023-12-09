KERNEL_SRC := /usr/src/linux-hwe-6.2-headers-6.2.0-37
CFLAGS += -msse2

obj-m += core_mode.o

KDIR = /lib/modules/$(shell uname -r)/build


all:
	make -C $(KDIR)  M=$(shell pwd) modules

clean:
	make -C $(KDIR)  M=$(shell pwd) clean