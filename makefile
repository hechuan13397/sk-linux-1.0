
include cfg.mk

PETALINUX_STAGE=$(ROOT_DIR)/petalinux/pre-build/linux/rootfs/stage


#the toppest make commands

.PHONY: build prepare patch all clean distclean install purge help
all: boot kernel rootfs component
build: prepare patch config
prepare: boot-prepare kernel-prepare rootfs-prepare component-prepare 
patch: boot-patch kernel-patch rootfs-patch component-patch 
config: rootfs-config component-config
clean: boot-clean kernel-clean rootfs-clean component-clean
distclean: boot-distclean kernel-distclean rootfs-distclean component-distclean
install: boot-install kernel-install rootfs-install  component-install rootbox-install 
purge: purge-confirm boot-purge kernel-purge rootfs-purge component-purge drv-purge rootbox-purge 

.PHONY: purge-confirm
purge-confirm:
	read -p "Are you sure you to rm all project[yes/no]?" REPLY; \
	if [ "$$REPLY" != "yes" ]; then \
		exit 1; \
	else \
		echo "continue to start"; \
	fi

#====================================================================================
#                          boot
#====================================================================================

.PHONY: boot-prepare boot-patch boot boot-install boot-clean boot-distclean boot-purge boot-img
boot-prepare:
	test -d $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT) || tar -xvf $(TOP_DIR)/source/$(BOOT_DIR).tar.gz -C $(TOP_DIR)/uboot; \
    	cd uboot; \
	if [ -d $(BOOT_DIR) ]; then \
		mv $(BOOT_DIR) $(BOOT_DIR_SHORT); \
	fi
boot-git-pre:

boot-patch:
	cp -arvf $(PATCH_DIR)/uboot/. $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT)
boot:
	make -C $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT)

#this is only for test. 
#it's need to set nfs rootfs/home dir to copy boot to sd 
boot-img:
	cd img/; \
	./boot-make.sh

boot-install:

boot-clean:
	make -C $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT) clean
boot-distclean:
	make -C $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT) distclean
boot-purge:
	rm -rf $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT)
#====================================================================================
#                          kernel
#====================================================================================
.PHONY: kernel-prepare kernel-patch kernel kernel-install kernel-clean kernel-distclean kernel-purge

kernel-prepare:
	test -d $(TOP_DIR)/kernel/$(KERNEL_BRA) || unzip $(TOP_DIR)/source/$(KERNEL_BRA)-$(KERNEL_COMMIT).zip -d $(TOP_DIR)/kernel;
kernel-patch:
	cp -arvf $(PATCH_DIR)/kernel/. $(TOP_DIR)/kernel/$(KERNEL_BRA)
kernel:
	make -C $(TOP_DIR)/kernel/$(KERNEL_BRA) -j 4

kernel-install:

kernel-clean:
	make -C $(TOP_DIR)/kernel/$(KERNEL_BRA) clean
kernel-distclean:
	make -C $(TOP_DIR)/kernel/$(KERNEL_BRA) distclean
kernel-purge:
	rm -rf $(TOP_DIR)/kernel/$(KERNEL_BRA)


#====================================================================================
#                          rootfs
#====================================================================================
.PHONY: rootfs-prepare rootfs-patch rootfs-config  rootfs  rootfs-clean rootfs-distclean  \
    	rootfs-install  rootfs-uninstall rootfs-purge
#++++++++++++++++++++++++++++++++++++++++++++++++++++
rootfs-prepare:
	make -C $(ROOTFS_SRC_DIR) prepare
rootfs-patch:
	make -C $(ROOTFS_SRC_DIR) patch
rootfs-config:
	make -C $(ROOTFS_SRC_DIR) config
rootfs:
	make -C $(ROOTFS_SRC_DIR) all
rootfs-install:
	make -C $(ROOTFS_SRC_DIR) install
rootfs-clean:
	make -C $(ROOTFS_SRC_DIR) clean
rootfs-distclean:
	make -C $(ROOTFS_SRC_DIR) distclean
rootfs-purge:
	make -C $(ROOTFS_SRC_DIR) purge

#===================================================================================
#			skdrv
#===================================================================================
.PHONY: drv drv-install drv-clean drv-purge

drv:
	make -C $(SKDRV_DIR) all

drv-install:
	mkdir -p $(ROOT_DEVEL_DIR)/usr/sklib
	make -C $(SKDRV_DIR) install

drv-clean:
	make -C $(SKDRV_DIR) clean

drv-purge: drv-clean

#====================================================================================
#                          rootbox
#====================================================================================
.PHONY: rootfs-cp rootbox-install rootbox-purge 

#cp syslib and usrlib to pub rootbox
#install install and petalinux-prebuild dir to pub-rootbox
rootfs-cp:
	mkdir -p $(PUB_ROOTBOX)
	cd $(PUB_ROOTBOX);\
	mkdir -p home/root bin sbin lib usr/lib dev etc/init.d mnt opt proc root sys tmp var
	rsync -arv $(ROOTBOX_PATCH_DIR)/etc $(PUB_ROOTBOX)/
	rsync -arv --exclude=*.a --exclude=*.prl --exclude=depmod.d --exclude=modprobe.d --exclude=pkgconfig \
	    --exclude=security --exclude=udev $(PETALINUX_STAGE)/lib/ $(PUB_ROOTBOX)/lib/
	rsync -arv $(PETALINUX_STAGE)/usr/lib/ $(PUB_ROOTBOX)/usr/lib/
	rsync -arv --exclude=*.la --exclude=cmake --exclude=udev --exclude=*.a --exclude=*.prl\
	    --exclude=print-camera* $(SYS_ROOT_DIR)/install/lib/ $(PUB_ROOTBOX)/usr/lib/
	rsync -arv $(SYS_ROOT_DIR)/install/plugins $(PUB_ROOTBOX)/usr/lib/
	#start to install include and lib for app develop
	mkdir -p $(ROOT_DEVEL_DIR)/usr/lib
	rsync -arv $(PUB_ROOTBOX)/lib $(ROOT_DEVEL_DIR)/
	rsync -arv $(PUB_ROOTBOX)/usr/lib $(ROOT_DEVEL_DIR)/usr/
	rsync -arv $(SYS_ROOT_DIR)/install/include $(ROOT_DEVEL_DIR)/usr/
	rsync -arv $(PETALINUX_STAGE)/usr/include/ $(ROOT_DEVEL_DIR)/usr/include/

rootbox-install: rootfs-install rootfs-cp drv-install

rootbox-purge:
	rm -rf $(PUB_ROOTBOX)
	rm -rf $(ROOT_DEVEL_DIR)

#====================================================================================
#                         component
#====================================================================================
.PHONY:  component-prepare component-config component-patch component  component-clean  component-install \
    	 component-purge component-test
#++++++++++++++++++++++++++++++++++++++++++++++++++++

component-prepare:
	make -C $(COMPONENT_DIR) prepare
component-config:
	make -C $(COMPONENT_DIR) config
component-patch:
	make -C $(COMPONENT_DIR) patch
component:
	make -C $(COMPONENT_DIR) all
component-clean:
	make -C $(COMPONENT_DIR) clean
component-install:
	make -C $(COMPONENT_DIR) install
component-uninstall:
	make -C $(COMPONENT_DIR) uninstall
component-purge:
	make -C $(COMPONENT_DIR) purge
component-test:
	make -C $(COMPONENT_DIR) test
	
.PHONY: nfs-install
nfs-install:
	sudo rsync -arv $(PUB_ROOTBOX)  $(NFS_BOOT)/
	sudo chown -R root:root $(NFS_BOOT)/rootbox

help:
	@echo "make build . prepare and config all project"
	@echo "make all   . make all project"
	@echo "make config . config all project"
	@echo "make patch  . do all patches. all modified files in patch dir"
	@echo "make install . install all share lib that was complied before to pub/rootbox"
	@echo "make purge   . rm all source project. do it carefully. it would rm all workspace."
	@echo "make clean distclean. to clean all project. it's little usage."	
