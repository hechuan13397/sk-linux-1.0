
#global variable
ifeq ($(ROOT_DIR), )
    $(error "please source env-setup.sh in root dir")
endif
$(warning rootdir = $(ROOT_DIR))

TOP_DIR=$(ROOT_DIR)
ROOTFS_SRC_DIR=${ROOT_DIR}/rootfs
PUB_ROOTBOX=${ROOT_DIR}/pub/rootbox
PUB_IMG_DIR=${ROOT_DIR}/pub/img
ROOT_DEVEL_DIR=$(ROOT_DIR)/pub/sk-devel
COMPONENT_DIR=${ROOT_DIR}/component
SKDRV_DIR=$(ROOT_DIR)/sk-drv
PATCH_DIR=${ROOT_DIR}/patch
ROOTBOX_PATCH_DIR=$(PATCH_DIR)/rootfs
SYS_ROOT_DIR=$(ROOT_DIR)/sys-root
SOURCE_DIR=$(ROOT_DIR)/source
PETALINUX_DIR=$(ROOT_DIR)/petalinux

#boot directory 
#the long dir is kept as original that was downloaded from digilent offcial site
BOOT_DIR=u-boot-digilent-digilent-v2016.07
BOOT_DIR_SHORT=digilent-v2016.07

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
