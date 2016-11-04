#!/bin/sh

#make
echo "$1"
if [ $1 -eq 1 ]; then
	make uImage LOADADDR=0x300000
fi
sudo cp -vf  arch/arm/boot/uImage /home/hechuan/work/tftp 
#sudo cp -vf drivers/media/i2c/soc_camera/mt9m001.ko /home/hechuan/work/nfsboot/rootfs-vid-cap//lib/modules/4.6.0-g35c0d02-dirty/kernel/drivers/media/i2c/soc_camera/
#sudo cp -vf drivers/media/platform/xilinx/*.ko /home/hechuan/work/nfsboot/rootfs-vid-cap//lib/modules/4.6.0-g35c0d02-dirty/kernel/drivers/media/platform/xilinx/
#sudo cp -vf  drivers/gpu/drm/i2c/adv7511.ko /home/hechuan/work/nfsboot/xylon-trd-2014-4/home/root
