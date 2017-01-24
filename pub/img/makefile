
include ../../cfg.mk


#boot env txt. 
#bootenv.img can be download by uboot when uboot boot up to cmdline.
#uboot init first command line is in uEnv.txt that must be put in sd cards.
BOOT_ENV=bootenv
BOOT_ENV_TXT=$(BOOT_ENV).txt
BOOT_ENV_IMG=$(BOOT_ENV).img
BOOT_COMPONENT_DIR=$(PUB_IMG_DIR)/plbit

PL_SUFFIX=v1.0
FSBL=$(PUB_IMG_DIR)/plbit/fsbl-plbit-$(PL_SUFFIX)/fsbl.elf
SYSBIT=$(PUB_IMG_DIR)/plbit/fsbl-plbit-$(PL_SUFFIX)/system.bit

KERNEL_DTB_DIR=kernel-dtb
LOGO_NAME=sk_log.png

#petalinux project directory
PETA_PROJECT_TMP=$(PETALINUX_DIR)/peta-zynq-tmp
.PHONY:boot_env boot_img dtb dts kernel_logo sd_ramdisk_img

boot_env:
	mkimage -T script -C none -n 'Demo Script File' -d $(BOOT_ENV_TXT) $(BOOT_ENV_IMG)
	cp -vf $(BOOT_ENV_IMG) $(TFTP_HOST)

boot_img:
	cp -vf ../../uboot/$(BOOT_DIR_SHORT)/u-boot $(PUB_IMG_DIR)/plbit/u-boot.elf
ifeq ($(CFG_BOOT_WITH_BIT),yes)
	petalinux-package --boot --force --format BIN --fsbl $(FSBL) --fpga $(SYSBIT) --uboot $(PUB_IMG_DIR)/plbit/u-boot.elf -o BOOT.bin -p $(PETA_PROJECT_TMP)
else
	petalinux-package --boot --force --format BIN --fsbl $(FSBL) --uboot $(PUB_IMG_DIR)/plbit/u-boot.elf -o BOOT.bin -p $(PETA_PROJECT_TMP)
endif
	sudo cp -vf BOOT.bin $(NFS_BOOT)/rootbox/home/root/
	
dtb:
	make -C $(KERNEL_DTB_DIR) dtb
dts:
	make -C $(KERNEL_DTB_DIR) dts

rootfs_img:
	mkfs.ubifs -m 2048 -e 126KiB -c 500 -r $(PUB_ROOTBOX)/lib ubifs.img

sd_ramdisk_img:
	#create empty ramdisk.image kbyte image size 70M
	dd if=/dev/zero of=ramdisk.image bs=1024 count=71680
	mke2fs -F ramdisk.image -L "ramdisk" -b 1024 -m 0
	tune2fs ramdisk.image -i 0
	chmod a+rwx ramdisk.image
	mkdir -p ramdisk
	sudo mount -o loop ramdisk.image ramdisk/
	#cp file to rootfs.then generate img file
	sudo cp -avr $(PUB_ROOTBOX)/* ramdisk/
	sudo chown -R root:root ramdisk/
	sudo umount ramdisk/ -f
	rm ramdisk -rf
	gzip -9 ramdisk.image
	mkimage -A arm -T ramdisk -C gzip -d ramdisk.image.gz uramdisk.image.gz
	rm ramdisk.image.gz
	#mv uramdisk.image.gz uramdisk72M.image.gz
		
kernel_logo:
	pngtopnm $(LOGO_NAME) | ppmquant 224 | pnmtoplainpnm > logo_linux_clut224.ppm

