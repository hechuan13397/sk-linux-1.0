#cross compiler define
CROSS=arm-linux-gnueabihf-
CC=${CROSS}gcc
CC_PLUS=${CROSS}g++
AR=${CROSS}ar
LD=${CROSS}ld
OC=${CROSS}objcopy
RANLIB=${CROSS}ranlib
RM=rm

#global variable
ifeq ($(ROOT_DIR), )
    $(error "please source env-setup.sh in root dir")
endif
#$(warning rootdir = $(ROOT_DIR))

#some project config define
#gen boot with plbit
CFG_BOOT_WITH_BIT=yes

#compiling with linux printer support
CFG_PRINTER_SUPPORT=yes
#opencv support
CFG_OPENCV_SUPPORT=yes 
#ffmpeg for video process
CFG_FFMPEG_SUPPORT=yes

#to silent receipt echo
Q=@

BUILDROOT_DIR=${ROOT_DIR}/buildroot-2016.11
TOP_DIR=$(ROOT_DIR)
ROOTFS_SRC_DIR=${ROOT_DIR}/rootfs
PUB_ROOTBOX=${ROOT_DIR}/pub/rootbox
BUILDROOT_TARGET=${BUILDROOT_DIR}/output/target/
PUB_IMG_DIR=${ROOT_DIR}/pub/img
ROOT_DEVEL_DIR=$(ROOT_DIR)/pub/sk-devel
BUILDROOT_STAGE=${BUILDROOT_DIR}/output/staging
COMPONENT_DIR=${ROOT_DIR}/component
SKDRV_DIR=$(ROOT_DIR)/sk-drv
PATCH_DIR=${ROOT_DIR}/patch
ROOTBOX_PATCH_DIR=$(PATCH_DIR)/rootfs
SOURCE_DIR=$(ROOT_DIR)/source
PETALINUX_DIR=$(ROOT_DIR)/petalinux

#boot directory 
#the long dir is kept as original that was downloaded from digilent offcial site

#BOOT_DIR=u-boot-digilent-digilent-v2016.07
#BOOT_DIR_SHORT=digilent-v2016.07

BOOT_DIR=u-boot-xlnx-xilinx-v2016.3
BOOT_DIR_SHORT=xilinx-v2016.3

BOOT_PAT_DIR=$(PATCH_DIR)/uboot/$(BOOT_DIR_SHORT)

#kernel branch from analog
KERNEL_BRA=linux-xcomm_zynq
KERNEL_COMMIT=183c266

#tftp server export file.
#uboot,kernel image,dtb, can be put into this directory
TFTP_HOST=~/work/tftp

#rootbox can be copied into this directory.
#kernel nfs root config must be turned on.
#here,define your nfs debug directory
NFS_BOOT=~/work/nfsboot
