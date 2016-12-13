

include ../../cfg.mk

CROSS=arm-linux-gnueabihf-
CC=${CROSS}gcc
CC_PLUS=${CROSS}g++
AR=${CROSS}ar
LD=${CROSS}ld
OC=${CROSS}objcopy
RANLIB=${CROSS}ranlib
RM=rm

ASFLAGS=

OBJ_DIR=obj/
DEPEND_DIR=depend/
MOD_NAME=v4l2cap

MOD_NAME_LIB=libsk_$(MOD_NAME).a

#some string manuals
MOD_OBJS = $(patsubst %.c,%.o,$(wildcard *.c))
DEP_FILES = $(patsubst %.c,%.d,$(wildcard *.c))

BUILD_OBJS = $(addprefix $(OBJ_DIR),$(MOD_OBJS))
BUILD_DEPS = $(addprefix $(DEPEND_DIR),$(DEP_FILES))

KERNEL_PATH = $(ROOT_DIR)/kernel/$(KERNEL_BRA)

INCDIR = -I$(ROOT_DEVEL_DIR)/usr/include
INCDIR += -I$(KERNEL_PATH)/include/uapi
INCDIR += -I$(KERNEL_PATH)/include
INCDIR += -I./include

PUBINC = sk_$(MOD_NAME).h
PUB_STATICLIB = libsk_$(MOD_NAME).a

CFLAGS= -c -Os -g -fomit-frame-pointer -pipe -fno-common -fno-builtin -Wall

.PHONY: all clean
all: $(MOD_NAME_LIB)
	

$(MOD_NAME_LIB): $(BUILD_OBJS)
	@echo build lib.... file: $@
	@$(AR) $(ASFLAGS) crs $@ $^
	#@$(RANLIB) $@

$(BUILD_DEPS): $(DEPEND_DIR)%.d: %.c
	mkdir -p $(DEPEND_DIR)
	@echo make depend file: $@
	@$(CC) -MM $(CFLAGS) $< $(INCDIR) > $@.$$$
	@sed "s,\([^.]*\).o:,$(OBJ_DIR)\1.o $(DEPEND_DIR)\$(*).d:," $@.$$$ > $@
	@$(RM) -f $@.$$$

$(BUILD_OBJS): $(OBJ_DIR)%.o : %.c
	mkdir -p $(OBJ_DIR)
	@echo '  [CC] : $@'
	@$(CC) $(CFLAGS) $(INCDIR) -o $@  -c $<

install: $(MOD_NAME_LIB)
	@cp -vf $(PUBINC) $(ROOT_DEVEL_DIR)/usr/include/
	@cp -vf $(PUB_STATICLIB) $(ROOT_DEVEL_DIR)/usr/sklib/
clean:
	$(RM) -f $(BUILD_DEPS)
	$(RM) -f $(BUILD_OBJS)

include $(BUILD_DEPS)

