#!/bin/sh

LINARO_GCC=gcc-linaro-4.9-2016.02-x86_64_arm-linux-gnueabihf

if [ ! -d /opt/tools/$LINARO_GCC ];then
	sudo tar -xvf tools/$LINARO_GCC.tar.xz -C /opt/tools/
fi

sudo cp -vf tools/gitbackup.sh /usr/sbin/
sudo apt-get install device-tree-compiler autopoint libtool cmake u-boot-tools \
    lib32ncurses5 lib32z1 lib32stdc++6 libncurses5-dev mtd-utils texinfo

echo "please add dir: /opt/tools/$LINARO_GCC/bin to your \$PATH
	in /etc/profile or ~/bash.rc"
echo "such as export PATH=/opt/tools/$LINARO_GCC/bin:\$PATH"
