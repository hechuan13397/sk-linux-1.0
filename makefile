
include cfg.mk

define GIT_PREPARE =
git init ;\
git add . ;\
git add .config -f ;\
git commit -m "init version" ;\
git config core.fileMode false ;
endef

#the toppest make commands

.PHONY: prepare all clean distclean install help
all: boot kernel buildroot drv
clean: boot-clean kernel-clean buildroot-clean drv-clean
install: boot-install kernel-install drv-install

#====================================================================================
#                          boot
#====================================================================================

.PHONY: boot-prepare boot-git-pre boot-patch boot boot-install boot-clean boot-distclean boot-img
boot-prepare:
	test -d $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT) || tar -xvf $(TOP_DIR)/source/$(BOOT_DIR).tar.gz -C $(TOP_DIR)/uboot; \
    	cd uboot; \
	if [ -d $(BOOT_DIR) ]; then \
		mv $(BOOT_DIR) $(BOOT_DIR_SHORT); \
	fi
boot-git-pre:
	if [ ! -d uboot/$(BOOT_DIR_SHORT)/.git ];then\
	 	cd uboot/$(BOOT_DIR_SHORT) ;\
		$(GIT_PREPARE) \
	fi

boot-patch:
	cp -arvf $(BOOT_PAT_DIR)/. $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT)
boot:
	make -C $(TOP_DIR)/uboot/$(BOOT_DIR_SHORT) -j 4

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
#====================================================================================
#                          kernel
#====================================================================================
.PHONY: kernel-prepare kernel-git-pre  kernel-patch kernel kernel-install kernel-clean kernel-distclean 

kernel-prepare:
	test -d $(TOP_DIR)/kernel/$(KERNEL_BRA) || unzip $(TOP_DIR)/source/$(KERNEL_BRA)-$(KERNEL_COMMIT).zip -d $(TOP_DIR)/kernel;
kernel-git-pre:
	if [ ! -d kernel/$(KERNEL_BRA)/.git ];then\
	 	cd kernel/$(KERNEL_BRA);\
		$(GIT_PREPARE) \
	fi
kernel-patch:
	cp -arvf $(PATCH_DIR)/kernel/. $(TOP_DIR)/kernel/$(KERNEL_BRA)

kernel:
	make -C $(TOP_DIR)/kernel/$(KERNEL_BRA) -j 4

kernel-install:

kernel-clean:
	make -C $(TOP_DIR)/kernel/$(KERNEL_BRA) clean
kernel-distclean:
	make -C $(TOP_DIR)/kernel/$(KERNEL_BRA) distclean

#===================================================================================
#			skdrv
#===================================================================================
.PHONY: drv drv-install drv-clean 

drv:
	make -C $(SKDRV_DIR) all

drv-install:
	mkdir -p $(BUILDROOT_STAGE)/usr/sklib
	make -C $(SKDRV_DIR) install

drv-clean:
	make -C $(SKDRV_DIR) clean

#====================================================================================
#                         buildroot-2016.11
#====================================================================================
.PHONY:  buildroot buildroot-clean
#++++++++++++++++++++++++++++++++++++++++++++++++++++
buildroot:
	make -C $(BUILDROOT_DIR)
buildroot-clean:
	make -C $(BUILDROOT_DIR) clean

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
	@echo "make clean distclean. to clean all project. it's little usage."	
