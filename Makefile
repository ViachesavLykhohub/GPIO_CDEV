TARGET := hw2
INSTALL_PATH := /srv/nfs/busybox/test_modules

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m   := $(addsuffix .o, $(TARGET))
ccflags-y := -g 
else

# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) -C $(KDIR) M=$$PWD
	cp $(addsuffix .ko, $(TARGET)) $(addsuffix .ko.unstripped, $(TARGET))
	$(CROSS_COMPILE)strip -g $(TARGET).ko

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

install: $(INSTALL_PATH)
	sudo cp ./$(addsuffix .ko, $(TARGET)) $<

$(INSTALL_PATH):
	sudo mkdir $@

%.i %.s : %.c
	$(ENV_CROSS) \
	$(MAKE) -C $(KDIR) M=$$PWD $@

endif
