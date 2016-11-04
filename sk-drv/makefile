
include ../cfg.mk

CROSS=arm-linux-gnueabihf-
CC=${CROSS}gcc
CC_PLUS=${CROSS}g++
AR=${CROSS}ar
LD=${CROSS}ld
OC=${CROSS}objcopy
RANLIB=${CROSS}ranlib
RM=rm

DEPEND_DIR = depend
OBJ_DIR = obj


CFLAGS= -c -Os -g -fomit-frame-pointer -pipe -fno-common -fno-builtin -Wall
INCDIR += -I./v4l2cap
MAIN_INSTALL = $(NFS_BOOT)/rootbox/home/root

LIBPATH = -L/home/hechuan/work/github/sk-linux-1.0/pub/rootbox/usr/lib
LIBPATH += -L./v4l2cap
SYS_LIBS = -lrt  -lmediactl -lv4l2 -lv4l2subdev -lv4lconvert -ljpeg -lpthread -lstdc++ -lc -lm -lz
SYS_LIBS += -lsk_v4l2cap
LIB = -Wl,--start-group $(SYS_LIBS) -Wl,--end-group

OBJS = sk_v4l2test

$(OBJS): drv $(OBJS).o 
	$(CC_PLUS) $(LIBPATH) -o $@ $(OBJS).o $(LIB)
	sudo cp -vf $(OBJS)  $(MAIN_INSTALL)
	
.PHONY: all install clean 

all:
	make -C v4l2cap
install: 
	make -C v4l2cap install
clean: 
	rm -rf v4l2cap/$(DEPEND_DIR) v4l2cap/$(OBJ_DIR)

%.o : %.c
	$(CC) -c -g $(CFLAGS) $(INCDIR) $< -o $@
