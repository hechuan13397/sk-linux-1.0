
include ../cfg.mk


DEPEND_DIR = depend
OBJ_DIR = obj

CFLAGS= -c -Os -g -fomit-frame-pointer -pipe -fno-common -fno-builtin -Wall
INCDIR += -I./v4l2cap
MAIN_INSTALL = $(NFS_BOOT)/rootbox/root

LIBPATH = -L$(BUILDROOT_TARGET)/usr/lib
LIBPATH += -L./v4l2cap
SYS_LIBS = -lsk_v4l2cap -lmediactl -lv4l2subdev -ludev
LIB = -Wl,--start-group $(SYS_LIBS) -Wl,--end-group

OBJS = sk_v4l2test sk_uiotest
UIO_TEST=sk_uiotest
V4L2_TEST=sk_v4l2test

all_test: $(OBJS)

$(V4L2_TEST): $(V4L2_TEST).o 
	$(CC_PLUS) $(LIBPATH) -o $@ $< $(LIB)
	sudo cp -vf $@  $(MAIN_INSTALL)

$(UIO_TEST): $(UIO_TEST).o 
	$(CC_PLUS) $(LIBPATH) -o $@ $< $(LIB)
	sudo cp -vf $@  $(MAIN_INSTALL)
	
.PHONY: all install clean 

all:
	make -C v4l2cap
install: 
	make -C v4l2cap install
clean: 
	rm -f $(OBJS) *.o
	rm -rf v4l2cap/$(DEPEND_DIR) v4l2cap/$(OBJ_DIR)

%.o : %.c
	$(CC) -c -g $(CFLAGS) $(INCDIR) $< -o $@
