
include ../../cfg.mk


#boot env txt. 
#bootenv.img can be download by uboot when uboot boot up to cmdline.
#uboot init first command line is in uEnv.txt that must be put in sd cards.
BOOT_ENV=bootenv
BOOT_ENV_TXT=$(BOOT_ENV).txt
BOOT_ENV_IMG=$(BOOT_ENV).img
BOOT_COMPONENT_DIR=$(PUB_IMG_DIR)/plbit
KERNEL_DTB_DIR=kernel-dtb

#petalinux project directory
PETA_PROJECT_TMP=$(PETALINUX_DIR)/peta-zynq-tmp
.PHONY:boot_env boot_img dtb dts
boot_env:
	mkimage -T script -C none -n 'Demo Script File' -d $(BOOT_ENV_TXT) $(BOOT_ENV_IMG)
	cp -vf $(BOOT_ENV_IMG) $(TFTP_HOST)

boot_img:
	cp -vf ../../uboot/$(BOOT_DIR_SHORT)/u-boot $(BOOT_COMPONENT_DIR)/u-boot.elf
	petalinux-package --boot --force --format BIN --fsbl $(BOOT_COMPONENT_DIR)/SK_VDMA.elf --fpga $(BOOT_COMPONENT_DIR)/system_top.bit --uboot $(BOOT_COMPONENT_DIR)/u-boot.elf -o BOOT.bin -p $(PETA_PROJECT_TMP)
	sudo cp -vf BOOT.bin $(NFS_BOOT)/rootbox/home/root/
	
dtb:
	make -C $(KERNEL_DTB_DIR) dtb
dts:
	make -C $(KERNEL_DTB_DIR) dts

rootfs_img:
	mkfs.ubifs -m 2048 -e 126KiB -c 500 -r $(PUB_ROOTBOX) ubifs.img
		
