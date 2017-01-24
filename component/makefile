
#makefile for component submodule

include ../cfg.mk

#qt variable
QT_DIR = qt-everywhere-opensource-src-5.6.0
QMAKE_BIN = $(COMPONENT_DIR)/$(QT_DIR)/qtbase/bin/qmake
QWT_DIR = qwt-6.1.3

#ffmpeg variable 
FFMPEG_DIR = ffmpeg-3.1.3

#gphoto2 variable
GPHOTO_DIR = libgphoto2-2.5.10

#libtool and libusb variable
LIBTOOL_DIR = libtool-2.4.2
LIBUSB_DIR = libusb-1.0.18

#libtslib dir variable
TSLIB_DIR = tslib-1.1

GUTEN_DIR=gutenprint-5.2.11
#common unix print system
CUPS_DIR=cups-release-2.1.4

#variable asign by cfg
QT_VERBOSE=yes

MODULES= tslib qt qwt libtool libusb gphoto

ifeq ($(CFG_FFMPEG_SUPPORT),yes)
MODULES += ffmpeg
endif

ifeq ($(CFG_PRINTER_SUPPORT),yes)
MODULES += printer
endif

MODULES_PRE=$(addsuffix -prepare,$(MODULES))
MODULES_PAT=$(addsuffix -patch,$(MODULES))
MODULES_CFG=$(addsuffix -config,$(MODULES))
MODULES_INS=$(addsuffix -install,$(MODULES))
MODULES_CLEAN=$(addsuffix -clean,$(MODULES))
MODULES_DIST_CLEAN=$(addsuffix -distclean,$(MODULES))
MODULES_PURGE=$(addsuffix -purge,$(MODULES))

#all project do real make
.PHONY: $(MODULES)
all: $(MODULES)

printer: cups gutenprint

gutenprint: cups-install
	make -C $(GUTEN_DIR)
cups:
	make -C $(CUPS_DIR)
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
.PHONY: $(MODULES_PRE)
prepare: $(MODULES_PRE)

printer-prepare: cups-prepare gutenprint-prepare

cups-prepare:
	test -d $(CUPS_DIR) || tar -xvf $(SOURCE_DIR)/$(CUPS_DIR).tar.gz
gutenprint-prepare:
	test -d $(GUTEN_DIR) || tar -xvf $(SOURCE_DIR)/$(GUTEN_DIR).tar.bz2
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
.PHONY: $(MODULES_PAT)
patch: $(MODULES_PAT)

printer-patch: cups-patch gutenprint-patch
cups-patch:
	test ! -d $(PATCH_DIR)/buildroot-2016.11/package/$(CUPS_DIR) || \
	cp -rvf  $(PATCH_DIR)/buildroot-2016.11/package/$(CUPS_DIR)/. $(CUPS_DIR)/
gutenprint-patch:
	test ! -d $(PATCH_DIR)/buildroot-2016.11/package/$(GUTEN_DIR) || \
	cp -rvf  $(PATCH_DIR)/buildroot-2016.11/package/$(GUTEN_DIR)/. $(GUTEN_DIR)/
tslib-patch:
qt-patch:
	echo "$(PATCH_DIR)"
	test ! -d $(PATCH_DIR)/qt/$(QT_DIR) || \
	cp -rvf  $(PATCH_DIR)/qt/$(QT_DIR)/. $(QT_DIR)/
qwt-patch:
	echo "$(PATCH_DIR)"
	test ! -d $(PATCH_DIR)/qt/$(QWT_DIR) || \
	cp -rvf  $(PATCH_DIR)/qt/$(QWT_DIR)/. $(QWT_DIR)/
ffmpeg-patch:
libtool-patch:
libusb-patch:
gphoto-patch:

#all project config
.PHONY: $(MODULES_CFG)
config: $(MODULES_CFG)

printer-config: cups-config gutenprint-config

#--with-sysroot="$(INSTALL_DIR)"
#--prefix=$(INSTALL_DIR)
gutenprint-config:
	cd $(GUTEN_DIR); \
	aclocal && automake; \
	./configure --prefix=/usr \
	CFLAGS=-I$(INSTALL_DIR)/usr/include --host=$(CROSS_HOST) \
	LDFLAGS="-L$(INSTALL_DIR)/usr/lib -L$(INSTALL_DIR)/usr/lib" \
	--disable-libgutenprintui2 \
	--disable-samples \
	--without-doc \
	--without-gimp2 \
	--without-foomatic \
	--without-foomatic3 \
	--disable-escputil \
	--disable-test \
	--disable-testpattern \
    	--disable-cups-ppds \
	--disable-static

cups-config:
	cd $(CUPS_DIR); \
	aclocal  -I config-scripts && autoconf -I config-scripts; \
	./configure --host=$(CROSS_HOST) LDFLAGS=-L$(INSTALL_DIR)/usr/lib \
	--libdir=/usr/lib --disable-libusb --disable-static --disable-dbus --disable-systemd

tslib-config:
	cd $(TSLIB_DIR); \
	autoreconf -vfi; \
	./configure --prefix=$(INSTALL_DIR)/usr --host=$(CROSS_HOST)
libtool-config:
	cd $(LIBTOOL_DIR); \
	./configure --prefix=$(INSTALL_DIR)/usr --host=$(CROSS_HOST)
libusb-config:
	cd $(LIBUSB_DIR); \
	./configure --prefix=$(INSTALL_DIR)/usr --host=$(CROSS_HOST) --disable-udev

gphoto-config: libtool-install libusb-install
	cd $(GPHOTO_DIR); \
	autoreconf --install --symlink; \
	./configure --prefix=$(INSTALL_DIR)/usr --host=$(CROSS_HOST) \
	--with-camlibs=canon,ptp2 \
	LIBLTDL=$(INSTALL_DIR)/usr/lib/libltdl.so  LTDLINCL=-I$(INSTALL_DIR)/usr/include \
	LIBUSB1_LIBS=$(INSTALL_DIR)/usr/lib/libusb-1.0.so  LIBUSB1_CFLAGS=-I$(INSTALL_DIR)/usr/include/libusb-1.0

ffmpeg-config:
	cd $(FFMPEG_DIR); \
	./configure --prefix=$(INSTALL_DIR)/usr --enable-cross-compile --cross-prefix=${CROSS_COMPILE} --arch=armel --target-os=linux \
    	--disable-indev=alsa --disable-outdev=alsa --disable-static --enable-shared --disable-ffprobe --disable-ffserver \
	--disable-avfilter --disable-encoders --disable-hwaccels --disable-muxers --disable-bsfs --disable-protocols \
	--disable-outdevs --disable-filters --disable-parsers 

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
	-prefix $(INSTALL_DIR)/usr \
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
	-I$(INSTALL_DIR)/usr/include \
	-L$(INSTALL_DIR)/usr/lib \
	$(QT_LOG)

qwt-config:
	cd $(QWT_DIR);\
	$(QMAKE_BIN) qwt.pro
	
#all project install
.PHONY: $(MODULES_INS) 
install: $(MODULES_INS)
	
printer-install: cups-install gutenprint-install
gutenprint-install:
	make -C $(GUTEN_DIR) install DESTDIR=$(INSTALL_DIR)
cups-install:
	make -C $(CUPS_DIR) install DESTDIR=$(INSTALL_DIR) DSTROOT=$(INSTALL_DIR)
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
.PHONY: $(MODULES_CLEAN) 
clean: $(MODULES_CLEAN) 

printer-clean:cups-clean gutenprint-clean
cups-clean:
	make -C $(CUPS_DIR) clean
gutenprint-clean:
	make -C $(GUTEN_DIR) clean
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
.PHONY: $(MODULES_DIST_CLEAN)
distclean: $(MODULES_DIST_CLEAN)

printer-distclean:cups-distclean gutenprint-distclean
cups-distclean:
	make -C $(CUPS_DIR) distclean
gutenprint-distclean:
	make -C $(GUTEN_DIR) distclean
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
.PHONY: $(MODULES_PURGE)
purge: $(MODULES_PURGE)

printer-purge:gutenprint-purge cups-purge

gutenprint-purge:
	rm -rf $(GUTEN_DIR)
cups-purge:
	rm -rf $(CUPS_DIR)
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

