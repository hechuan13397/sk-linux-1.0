
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <poll.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <linux/fb.h>


#define VDMA_BASEADDR_PHY		(0x83000000)
#define VIDEO_BASEADDR0  (0x2000000)
#define VIDEO_BASEADDR1  (0x3000000)
#define VIDEO_BASEADDR2  (0x4000000)
#define H_ACTIVE            800
#define V_ACTIVE            480
#define VTC_BASEADDR_PHY (0x83c00000)
#define MIZ702_VTC_S00_AXI_SLV_REG0_OFFSET 0
#define MIZ702_VTC_S00_AXI_SLV_REG1_OFFSET 4
#define MIZ702_VTC_S00_AXI_SLV_REG2_OFFSET 8
#define MIZ702_VTC_S00_AXI_SLV_REG3_OFFSET 12
#define MIZ702_VTC_S00_AXI_SLV_REG4_OFFSET 16
#define MIZ702_VTC_S00_AXI_SLV_REG5_OFFSET 20
#define MIZ702_VTC_S00_AXI_SLV_REG6_OFFSET 24
#define MIZ702_VTC_S00_AXI_SLV_REG7_OFFSET 28


#define UIO_SIZE "/sys/class/uio/uio0/maps/map0/size"
#define UIO_SIZE_VTC "/sys/class/uio/uio1/maps/map0/size"

#define TSC_EVENT "/dev/input/event0"

int main( int argc, char **argv ) {
	int uio_fd, uio_vtc_fd;
	unsigned int uio_size, uio_vtc_size;
	FILE *size_fp, *size_vtc_fp;
	void *base_address;
	void *base_vtc_address;

	int event_fd = 0;
	event_fd = open(TSC_EVENT, O_RDONLY);
	if (!event_fd){
		printf("open event_fd failed\n");
		return 0;
	}
	
	return 0;
	

	QV4l2Cap_CameraSet();

	printf("start to open uio0\n");
	uio_fd = open("/dev/uio0", O_RDWR);
	if (!uio_fd){
		printf("open uio0 failed\n");
		return 0;
	}
	
	uio_vtc_fd = open("/dev/uio1", O_RDWR);
	if (!uio_vtc_fd){
		printf("open uio0 failed\n");
		return 0;
	}

	
	printf("start to open UIO_SIZE\n");
	size_fp = fopen(UIO_SIZE, "r");
	if (!size_fp){
		printf("open size_fp failed\n");
		return 0;
	}

	size_vtc_fp = fopen(UIO_SIZE_VTC, "r");
	if (!size_vtc_fp){
		printf("open size_fp failed\n");
		return 0;
	}
	
	fscanf(size_fp, "0x%08X", &uio_size);
	printf("uio_size = 0x%x\n",uio_size);
	
	base_address = mmap(NULL, uio_size,PROT_READ | PROT_WRITE,MAP_SHARED, uio_fd, 0);
	
	fscanf(size_vtc_fp, "0x%08X", &uio_vtc_size);
	base_vtc_address = mmap(NULL, uio_vtc_size,PROT_READ | PROT_WRITE,MAP_SHARED, uio_vtc_fd, 0);
	
	printf("base_address = 0x%08x\n",base_address);

	printf("vdma version = 0x%08x\n",*(int *)(base_address+0x2c));

	*(int *)(base_address+0x30) = 0x4;
	*(int *)(base_address+0x30) = 0x8;
	*(int *)(base_address+0xAC) = VIDEO_BASEADDR0;
	*(int *)(base_address+0xAC+4) = VIDEO_BASEADDR0;
	*(int *)(base_address+0xAC+8) = VIDEO_BASEADDR0;
	*(int *)(base_address+0xA8) = 0x00000C80;
	*(int *)(base_address+0xA4) = H_ACTIVE*4;
	*(int *)(base_address+0x30) = 0x3;
	*(int *)(base_address+0xA0) = V_ACTIVE;
	*(int *)(base_address+0x00) = 0x4;
	*(int *)(base_address+0x00) = 0x8;
	*(int *)(base_address+0x05c) = VIDEO_BASEADDR0;
	*(int *)(base_address+0x060) = VIDEO_BASEADDR0;
	*(int *)(base_address+0x064) = VIDEO_BASEADDR0;
	*(int *)(base_address+0x058) = 0x00000C80;
	*(int *)(base_address+0x054) = H_ACTIVE*4;
	*(int *)(base_address+0x000) = 0x00000003;
	*(int *)(base_address+0x050) = V_ACTIVE;

	*(int *)(base_vtc_address+MIZ702_VTC_S00_AXI_SLV_REG1_OFFSET) = (1078<<16)|536;
	*(int *)(base_vtc_address+MIZ702_VTC_S00_AXI_SLV_REG2_OFFSET) = (22<<16)|11;
	*(int *)(base_vtc_address+MIZ702_VTC_S00_AXI_SLV_REG3_OFFSET) = (210<<16)|22;
	*(int *)(base_vtc_address+MIZ702_VTC_S00_AXI_SLV_REG4_OFFSET) = (46<<16)|23;
	*(int *)(base_vtc_address+MIZ702_VTC_S00_AXI_SLV_REG5_OFFSET) = 0x04;
	
	
	// Access to the hardware can now occur¡­.
	munmap(base_address, uio_size);
	munmap(base_address, uio_vtc_size);
}
