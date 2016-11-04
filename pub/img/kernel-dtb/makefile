
include ../../../cfg.mk


DTB_IMG=zed-adv7511
#config for dtb compile
#input dts file that we change with debug
DTB_INPUT_DTS=zynq-zed-adv7511-mod-mtm001-test.dts

#by this input file. we can review original dts file
DTB_INPUT_DTB=$(DTB_IMG).dtb

OUTPUT_DTB=$(DTB_IMG).dtb
OUTPUT_DTS=$(DTB_IMG).dts


.PHONY: dtb dts

dtb:
	dtc -O dtb -I dts -o $(OUTPUT_DTB) $(DTB_INPUT_DTS)
	cp -vf $(OUTPUT_DTB) ../
	cp -vf $(OUTPUT_DTB) $(TFTP_HOST)

dts:
	dtc -I dtb -O dts -o $(OUTPUT_DTS) $(DTB_INPUT_DTB)
