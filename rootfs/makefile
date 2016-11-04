

.PHONY: prepare patch config all install clean distclean purge

prepare: busybox_prepare
patch:
config: busybox_config
all: busybox  
install: busybox_install
clean:  busybox_clean
distclean: busybox_distclean
purge: busybox_purge

#busybox compiling
.PHONY: busybox_config busybox_prepare busybox busybox_install busybox_clean busybox_distclean busybox_purge
busybox_prepare:
	make -C busybox prepare
busybox_config:
	make -C busybox config
busybox:
	make -C busybox all
busybox_install:
	make -C busybox install
busybox_clean:
	make -C busybox clean
busybox_distclean:
	make -C busybox distclean
busybox_purge:
	make -C busybox purge
