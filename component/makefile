
#makefile for component submodule


include ../cfg.mk


#qt variable
QT_DIR = qt-everywhere-opensource-src-5.6.0
QMAKE_BIN = $(COMPONENT_DIR)/$(QT_DIR)/qtbase/bin/qmake
QWT_DIR = qwt-6.1.3
QT_INSTALL_DIR = $(ROOT_DIR)/sys-root/install
#PETA_LINUX_DIR = /home/hechuan/work/zynq/petalinux/test/Avnet-Digilent-ZedBoard-2016.2/build/linux/rootfs/stage
SYS_ROOT_DIR = $(ROOT_DIR)/sys-root


#ffmpeg variable 
FFMPEG_DIR = ffmpeg-3.1.3

#gphoto2 variable
GPHOTO_DIR = libgphoto2-2.5.10

#libtool and libusb variable
LIBTOOL_DIR = libtool-2.4.2
LIBUSB_DIR = libusb-1.0.18

#libtslib dir variable
TSLIB_DIR = tslib-1.1


#all project do real make
.PHONY: qt qwt ffmpeg libtool libusb gphoto
all: tslib qt qwt ffmpeg libtool libusb gphoto

qt: 
	make -C $(QT_DIR) -j 4
qwt:
	make -C $(QWT_DIR) -j 4
ffmpeg:
	make -C $(FFMPEG_DIR) -j 4
libtool:
	make -C $(LIBTOOL_DIR)
libusb:
	make -C $(LIBUSB_DIR)
gphoto:
	make -C $(GPHOTO_DIR)
tslib:
	make -C $(TSLIB_DIR)
	
#all top make command
.PHONY: prepare config patch all install clean distclean purge test
#REPLY=?
test:
	echo "${PWD}/$(SOURCE_DIR)"
	@read -p "Are you sure you to rm all project[yes/no]?" REPLY; \
	if [ "$$REPLY" != "yes" ]; then \
		echo start to exit;\
	   	exit 1; \
	fi
	@echo "reply confirmed ok"

#all project prepare
.PHONY: tslib-prepare qt-prepare qwt-prepare ffmpeg-prepare libtool-prepare libusb-prepare gphoto-prepare
prepare: tslib-prepare qt-prepare qwt-prepare ffmpeg-prepare libtool-prepare libusb-prepare gphoto-prepare

ffmpeg-prepare:
	test -d $(FFMPEG_DIR) || tar -xvf $(SOURCE_DIR)/$(FFMPEG_DIR).tar.bz2
qt-prepare:
	test -d $(QT_DIR) || tar -xvf $(SOURCE_DIR)/$(QT_DIR).tar.gz
qwt-prepare:
	test -d $(QWT_DIR) || tar -xvf $(SOURCE_DIR)/$(QWT_DIR).tar.bz2
gphoto-prepare:
	test -d $(GPHOTO_DIR) || tar -xvf $(SOURCE_DIR)/$(GPHOTO_DIR).tar.bz2
libtool-prepare:
	test -d $(LIBTOOL_DIR) || tar -xvf $(SOURCE_DIR)/$(LIBTOOL_DIR).tar.gz
libusb-prepare:
	test -d $(LIBUSB_DIR) || tar -xvf $(SOURCE_DIR)/$(LIBUSB_DIR).tar.bz2
tslib-prepare:
	test -d $(TSLIB_DIR) || tar -xvf $(SOURCE_DIR)/$(TSLIB_DIR).tar.gz

#all project patch
.PHONY: qt-patch qwt-patch
patch: qt-patch qwt-patch

qt-patch:
	echo "$(PATCH_DIR)"
	test ! -d $(PATCH_DIR)/qt/$(QT_DIR) || \
	cp -rvf  $(PATCH_DIR)/qt/$(QT_DIR)/. $(QT_DIR)/

qwt-patch:
	echo "$(PATCH_DIR)"
	test ! -d $(PATCH_DIR)/qt/$(QWT_DIR) || \
	cp -rvf  $(PATCH_DIR)/qt/$(QWT_DIR)/. $(QWT_DIR)/

	
#all project config
.PHONY: tslib-config qt-config qwt-config ffmpeg-config libtool-config libusb-config gphoto-config
config: tslib-config qt-config qwt-config ffmpeg-config libtool-config libusb-config gphoto-config

tslib-config:
	cd $(TSLIB_DIR); \
	autoreconf -vfi; \
	./configure --prefix=$(INSTALL_DIR) --host=$(CROSS_HOST)
libtool-config:
	cd $(LIBTOOL_DIR); \
	./configure --prefix=$(INSTALL_DIR) --host=$(CROSS_HOST)
libusb-config:
	cd $(LIBUSB_DIR); \
	./configure --prefix=$(INSTALL_DIR) --host=$(CROSS_HOST) --disable-udev

gphoto-config: libtool-install libusb-install
	cd $(GPHOTO_DIR); \
	autoreconf --install --symlink; \
	./configure --prefix=$(INSTALL_DIR) --host=$(CROSS_HOST) \
	--with-camlibs=canon,ptp2 \
	LIBLTDL=$(INSTALL_DIR)/lib/libltdl.so  LTDLINCL=-I$(INSTALL_DIR)/include \
	LIBUSB1_LIBS=$(INSTALL_DIR)/lib/libusb-1.0.so  LIBUSB1_CFLAGS=-I$(INSTALL_DIR)/include/libusb-1.0

ffmpeg-config:
	cd $(FFMPEG_DIR); \
	./configure --prefix=$(INSTALL_DIR) --enable-cross-compile --cross-prefix=${CROSS_COMPILE} --arch=armel --target-os=linux \
    	--disable-indev=alsa --disable-outdev=alsa --disable-static --enable-shared --disable-ffprobe --disable-ffserver \
	--disable-avfilter --disable-encoders --disable-hwaccels --disable-muxers --disable-bsfs --disable-protocols \
	--disable-outdevs --disable-filters --disable-parsers 


QT_VERBOSE=yes

ifeq ($(QT_VERBOSE),yes)
QT_LOG=-verbose
else
QT_LOG=-silent
endif

#-qreal float
#-make libs \

qt-config: tslib-install
	cd $(QT_DIR);\
	./configure -xplatform xilinx-arm-linux-gnueabihf-g++ \
	-prefix $(QT_INSTALL_DIR) \
	-shared -no-pkg-config -opensource \
	-confirm-license -no-c++11 -no-openssl -no-xinput2 -no-xcb-xlib \
	-no-glib -no-mtdev -no-pch -no-dbus -no-xcb -no-directfb -no-kms \
	-optimized-qmake -no-nis -no-cups -no-iconv -no-gstreamer -no-system-proxies \
	-no-egl -no-eglfs -no-opengl -no-icu -no-fontconfig -no-qml-debug -qt-libpng -qt-libjpeg \
	-qt-freetype -qt-harfbuzz -qt-zlib -linuxfb -lrt -force-asserts -strip -tslib \
	-skip qtdoc \
	-skip qttranslations \
	-skip qtwebengine	\
	-skip qtenginio	\
	-skip qtscript	\
	-skip qttools \
	-skip qtgraphicaleffects \
	-skip qtcanvas3d \
	-skip qt3d \
	-skip qtdeclarative \
	-skip qtandroidextras \
	-skip qtwebchannel \
	-skip qtmacextras  \
	-skip qtwinextras \
	-skip qtlocation \
	-skip qtconnectivity \
	-skip qtmultimedia \
	-nomake tests \
	-nomake tools \
	-nomake examples \
	-I$(QT_INSTALL_DIR)/include \
	-L$(QT_INSTALL_DIR)/lib \
	$(QT_LOG)

qwt-config:
	cd $(QWT_DIR);\
	$(QMAKE_BIN) qwt.pro
	
#all project install
.PHONY: tslib-install qt-install qwt-install ffmpeg-install libtool-install libusb-install gphoto-install
install: tslib-install qt-install qwt-install ffmpeg-install libtool-install libusb-install gphoto-install
qt-install:
	make -C $(QT_DIR) install
qwt-install:
	make -C $(QWT_DIR) install
ffmpeg-install:
	make -C $(FFMPEG_DIR) install
libtool-install:
	make -C $(LIBTOOL_DIR) install
libusb-install:
	make -C $(LIBUSB_DIR) install
gphoto-install:
	make -C $(GPHOTO_DIR) install
tslib-install:
	make -C $(TSLIB_DIR) install
#all project clean
.PHONY: tslib-clean qt-clean qwt-clean ffmpeg-clean libtool-clean libusb-clean gphoto-clean
clean: tslib-clean qt-clean qwt-clean ffmpeg-clean libtool-clean libusb-clean gphoto-clean

qt-clean:
	make -C $(QT_DIR) clean
qwt-clean:
	make -C $(QWT_DIR) clean
ffmpeg-clean:
	make -C $(FFMPEG_DIR) clean
libtool-clean:
	-make -C $(LIBTOOL_DIR) clean
libusb-clean:
	make -C $(LIBUSB_DIR) clean
gphoto-clean:
	make -C $(GPHOTO_DIR) clean
tslib-clean:
	make -C $(TSLIB_DIR) clean

#all project distclean
.PHONY: qt-distclean qwt-distclean ffmpeg-distclean libtool-distclean libusb-distclean gphoto-distclean
distclean: qt-distclean qwt-distclean ffmpeg-distclean libtool-distclean libusb-distclean gphoto-distclean

qt-distclean:
	make -C $(QT_DIR) distclean
qwt-distclean:
	make -C $(QWT_DIR) distclean
ffmpeg-distclean:
	make -C $(FFMPEG_DIR) distclean
libtool-distclean:
	-make -C $(LIBTOOL_DIR) distclean
libusb-distclean:
	make -C $(LIBUSB_DIR) distclean
gphoto-distclean:
	make -C $(GPHOTO_DIR) distclean
tslib-distclean:
	make -C $(TSLIB_DIR) distclean
#all project purge
.PHONY: tslib-purge qt-purge qwt-purge ffmpeg-purge libtool-purge libusb-purge gphoto-purge
purge: tslib-purge qt-purge qwt-purge ffmpeg-purge libtool-purge libusb-purge gphoto-purge

tslib-purge:
	rm -rf $(TSLIB_DIR)	
qt-purge:
	rm -rf $(QT_DIR) 
qwt-purge:
	rm -rf $(QWT_DIR) 
ffmpeg-purge:
	rm -rf $(FFMPEG_DIR)
libtool-purge:
	rm -rf $(LIBTOOL_DIR)
libusb-purge:
	rm -rf $(LIBUSB_DIR)
gphoto-purge:
	rm -rf $(GPHOTO_DIR)

