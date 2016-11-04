

include ../../cfg.mk

CROSS_COMPILE	= arm-linux-gnueabihf-
BUSYBOX		:= busybox-1.25.0

.PHONY: prepare config all install clean distclean purge

prepare:
	test -d $(BUSYBOX) || tar -xvf $(BUSYBOX).tar.bz2
config:
	cd $(BUSYBOX);\
	make defconfig;\
	cp -vf $(PATCH_DIR)/busybox/config ./.config
all:
	cd $(BUSYBOX);\
	make 	
install:
	cd $(BUSYBOX);\
	make  CONFIG_PREFIX=$(PUB_ROOTBOX) install
clean: 
	cd $(BUSYBOX);\
	make  clean
distclean:
	cd $(BUSYBOX);\
	make  distclean
purge:
	rm -rf $(BUSYBOX)
