
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

#include "sk_v4l2cap.h"

static struct fb_var_screeninfo var;
static struct fb_fix_screeninfo fix;


static void setTTYCursor(int enable)
{
    const char * const devs[] = { "/dev/tty0", "/dev/tty", "/dev/console", 0 };
	const char * const *dev = NULL;
    int fd = -1;
    for (dev = devs; *dev; ++dev) {
        fd = open(*dev, O_RDWR);
        if (fd != -1) {
            // Enable/disable screen blanking and the blinking cursor.
            const char *termctl = enable ? "\033[9;15]\033[?33h\033[?25h\033[?0c" : "\033[9;0]\033[?33l\033[?25l\033[?1c";
            write(fd, termctl, strlen(termctl) + 1);
            close(fd);
			printf("tty device name = %s\n",*dev);
            return;
        }
    }
}

int open_fb(const char *dev)
{
	int fd = -1;
	fd = open(dev, O_RDWR);
	if (fd == -1) {
		printf("Error opening device %s : %s\n", dev, strerror(errno));
		exit(-1);
	}

	return fd;
}

#define ERROR(x) printf("fbtest error in line %s:%d: %s\n", __func__, \
	__LINE__, strerror(errno));


#define FBCTL(cmd, arg)				\
	if (ioctl(fd, cmd, arg) == -1) {	\
		ERROR("ioctl failed");		\
		exit(1); }
	

#define Tranverse32(X)                 ((((int)(X) & 0xff000000) >> 24) | \
                                                           (((int)(X) & 0x00ff0000) >> 8) | \
                                                           (((int)(X) & 0x0000ff00) << 8) | \
                                                           (((int)(X) & 0x000000ff) << 24))


#define Tranverse32_ex(X)                 ((((int)(X) & 0xff000000) >> 24) | \
                                                           (((int)(X) & 0x00ff0000) >> 16) | \
                                                           (((int)(X) & 0x0000ff00) >> 8) | \
                                                           (((int)(X) & 0x000000ff) << 24))


static void draw_pixel(void *fbmem, int x, int y, unsigned int color)
{
      if (var.bits_per_pixel == 8) {
		unsigned char *p;

		fbmem += fix.line_length * y;

		p = fbmem;

		p += x;

		*p = color;
	} else if (var.bits_per_pixel == 16) {
		unsigned short c;
		int r = (color >> 16) & 0xff;
		int g = (color >> 8) & 0xff;
		int b = (color >> 0) & 0xff;
		unsigned short *p;

		r = r * 8 / 5;
		g = g * 8 / 6;
		b = b * 8 / 5;

		c = (r << 11) | (g << 6) | (b << 0);

		fbmem += fix.line_length * y;

		p = fbmem;

		p += x;

		*p = c;
	} else if (var.bits_per_pixel == 24) {
		unsigned int *p;
		unsigned c;

		fbmem += fix.line_length * y;
		fbmem += 3 * x;

		p = fbmem;

        c = *p;
        c = (c & 0xFF000000) | (color & 0x00FFFFFF);

		*p = c;
	} else {
		unsigned int *p;

		fbmem += fix.line_length * y;

		p = fbmem;

		p += x;

		*p = color;
	}
}

int display_frame_on_fb0(unsigned char *input_frame,int frame_len,int width,int height,int stride)
{
	int i = 0,j=0;
	int rgbfra_len = frame_len*3/2;
	static int fd = -1;
	static unsigned char *pframe = NULL;
	//unsigned char *pframe = NULL;
	unsigned char *rgbframe = NULL;
	int *rgbframe_tmp = NULL;
	unsigned char *fbmem = NULL;
	
	if (-1 == fd){
		
		fd = open_fb("/dev/fb0");
		FBCTL(FBIOGET_VSCREENINFO, &var);
		FBCTL(FBIOGET_FSCREENINFO, &fix);
		
		pframe= mmap(0, var.yres_virtual * fix.line_length,PROT_WRITE | PROT_READ,MAP_SHARED, fd, 0);
		if (pframe == MAP_FAILED) {
			perror("mmap failed");
			exit(1);
		}
	}
#if 0
	printf("var.xres_virtual = %d var.yres_virtual = %d fix.line_length = %d bits_per_pixel = %d\n",var.xres_virtual,
	var.yres_virtual,fix.line_length,var.bits_per_pixel);
#endif

	rgbframe = malloc(height*stride*2+0x80); // now using yuv422
	//v4lconvert_yuyv_to_rgb24(input_frame,rgbframe,width,height,stride);
	rgbframe_tmp = (int *)input_frame;
	fbmem = pframe;
	//put argb image on fb
	printf("height = [%d], width = %d,var.bits_per_pixel = %d,fix.line_length = %d\n",height,width,var.bits_per_pixel,fix.line_length);
	for (i=0; i < height; i++){
		for (j = 0; j < width; j++){
			int pixel_invert = *rgbframe_tmp++;
			pixel_invert = Tranverse32(pixel_invert);
			//pixel_invert|=0xff000000;
			//pixel_invert <<= 8;
			draw_pixel(fbmem,j,i,pixel_invert);
		}
	}

	free(rgbframe);
	//memcpy(pframe,input_frame,frame_len);
	return 0;
}


//sample test
int main(int argc, char **argv)
{
	strFrameBuf  framebuf;
	QV4L2_RET ret = 0;
	int cnt = 0;

	setTTYCursor(0);
	
	QV4l2Cap_init();
	
	ret = QV4l2Cap_Start();
	if (ret != QV4L2_SUCCESS){
			printf("QV4l2Cap_Start failed\n");
			return 0;
	}

	QV4l2Cap_CameraSet(500);

	while (1){
		
		ret = QV4l2Cap_ObtainFrame(&framebuf);
		if (ret != QV4L2_SUCCESS){
			printf("obtain frame failed\n");
			continue;
		}

		display_frame_on_fb0(framebuf.frame_buff,framebuf.buff_length,800,480,800*4);
		
		printf("obtain frame ok cnt = %d\n",cnt++);
		//printf("framebuf.buff_length = %d\n",framebuf.buff_length);
		QV4l2Cap_ReleaseFrame();
	}
	return 0;
}


