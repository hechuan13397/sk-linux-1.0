#!/bin/sh

sudo mkdir -p /opt/tools
if [ ! -d /opt/tools/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux ];then
	sudo tar -xvf tools/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux.tar.xz -C /opt/tools/
fi

sudo cp -vf tools/gitbackup.sh /usr/sbin/
sudo apt-get install device-tree-compiler autopoint libtool cmake u-boot-tools \
    lib32ncurses5 lib32z1 lib32stdc++6 libncurses5-dev

echo "please add dir: /opt/tools/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin to your \$PATH
	in /etc/profile or ~/bash.rc"
echo "such as export PATH=/opt/tools/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:\$PATH"
