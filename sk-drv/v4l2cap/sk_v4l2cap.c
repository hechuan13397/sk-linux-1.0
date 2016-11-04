
/*
	camera video caputure sk api;
	created by hechuan@sunking.com
	date : 2016.11.1
	rev					changed 		fixed issue
	------------------------------------------------
	0x20161101	chuan.he		init version
*/


#include <linux/xilinx-v4l2-controls.h>

#include "common.h"
#include "sk_v4l2cap.h"
#include "mediactl_helper.h"
#include "v4l2_helper.h"


#define MOD_VERSION		"0x20161101"


#define POLL_TIMEOUT_MSEC 5000

#define INPUT_PIX_FMT  v4l2_fourcc('R','G','G','B')
#define COLOR_SPACE  V4L2_COLORSPACE_ADOBERGB
#define BYTES_PER_PIXEL 4
#define IMG_WIDTH 	(800)
#define IMG_HEIGHT 	(480)

#define XLNX_CAP_CARD_NAME "video_cap output 0"



//default video width and height setting as 800x480;
static int isInited = 0;
static struct v4l2_dev vid_dev;
static struct buffer *videobuf;


QV4L2_RET QV4l2Cap_init()
{
	memset(&vid_dev,0,sizeof(struct v4l2_dev));

	vid_dev.format.pixelformat = INPUT_PIX_FMT;
	vid_dev.format.bytesperline = BYTES_PER_PIXEL*IMG_WIDTH;
	vid_dev.format.colorspace = COLOR_SPACE;
	vid_dev.format.width = IMG_WIDTH;
	vid_dev.format.height = IMG_HEIGHT;
	vid_dev.buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vid_dev.mem_type = V4L2_MEMORY_MMAP;

	videobuf = NULL;
	
	printf("QV4L2 version = %s\n",MOD_VERSION);
	isInited = QV4L_TRUE;
	return QV4L2_SUCCESS;
}

void QV4l2Cap_SetWH(int w,int h)
{
	vid_dev.format.width = w;
	vid_dev.format.height = h;
	vid_dev.format.bytesperline = vid_dev.format.width*BYTES_PER_PIXEL;
	return;
}

void QV4l2Cap_SetImgFormat(QV4L2_CAP_IMG_FORMAT format)
{
	
	return;
}

QV4L2_RET QV4l2Cap_Start()
{
	int ret = 0,i = 0,j=0;
	CHECK_INIT();

	/* Configure media pipelines */
	ret = set_media_control(&vid_dev, MEDIA_NODE_0);
	if (ret < 0){
		QV4L2_ERR("failed\n");
		return QV4L2_FAIL;
	}

		/* Set v4l2 device name */
	ret = v4l2_parse_node(XLNX_CAP_CARD_NAME, vid_dev.devname);
	if (ret < 0){
		QV4L2_ERR("failed\n");
		return QV4L2_FAIL;
	}
	
	/* Initialize video input pipeline */
	ret = v4l2_init(&vid_dev, BUFFER_CNT);
	if (ret < 0){
		QV4L2_ERR("ret = %d\n",ret);
		return QV4L2_FAIL;
	}

	for (i = 0; i < BUFFER_CNT; ++i)  {
	 	struct v4l2_buffer buffer;
		memset(&buffer, 0, sizeof(buffer));
		buffer.type =V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		if (-1 == ioctl (vid_dev.fd, VIDIOC_QUERYBUF, &buffer)){			
			QV4L2_ERR("failed: %s\n", ERRSTR);
			return QV4L2_FAIL;
		}

		vid_dev.vid_buf[i].v4l2_buff_length=buffer.length;

		/* remember for munmap() */
		vid_dev.vid_buf[i].v4l2_buff = 
		mmap(NULL,buffer.length, PROT_READ|PROT_WRITE, MAP_SHARED,vid_dev.fd, buffer.m.offset);

		if (MAP_FAILED == vid_dev.vid_buf[i].v4l2_buff){
			/*if failed, unmap the previou mmaped memmory*/
			for (j=0; j < i; j++){
				munmap(vid_dev.vid_buf[j].v4l2_buff,vid_dev.vid_buf[j].v4l2_buff_length);
			}
			QV4L2_ERR("mmap failed: %s\n", ERRSTR);
			return QV4L2_FAIL;
		}
		vid_dev.vid_buf[i].index = i;
 	}

	/* Assigning buffer index and set exported buff handle */
	for(i=0;i<BUFFER_CNT;i++) {
		//Queue buffer to device
		v4l2_queue_buffer(&vid_dev,& (vid_dev.vid_buf[i]));
	}

	/* Start streaming */
	ret = v4l2_device_on(&vid_dev);
	if (ret < 0){
		QV4L2_ERR("ret = %d\n",ret);
		return QV4L2_FAIL;
	}

	QV4L2_DEG("ok\n");
	return QV4L2_SUCCESS;
}

QV4L2_RET QV4l2Cap_ObtainFrame(strFrameBuf *buf)
{
	int ret = 0;
	static int tpg_s2m_count = 0;
	struct pollfd fds = {.fd = vid_dev.fd, .events = POLLIN};

	CHECK_INIT();
	if (buf == NULL){
		return QV4L2_INVALID;
	}
	if (videobuf && videobuf->ishold){
		QV4L2_ERR("buffer not released\n");
		return QV4L2_FAIL;
	}
	if ((ret = poll(&fds, 1, POLL_TIMEOUT_MSEC)) > 0){
		if (fds.revents & POLLIN) {
				videobuf = v4l2_dequeue_buffer(&vid_dev,vid_dev.vid_buf);
				if (videobuf){
					buf->frame_buff = videobuf->v4l2_buff;
					buf->buff_length = videobuf->v4l2_buff_length;
					videobuf->ishold = QV4L_TRUE;
					return QV4L2_SUCCESS;
				}
		}	
	}
	QV4L2_ERR("failed\n");
	return QV4L2_FAIL;
}

QV4L2_RET QV4l2Cap_ReleaseFrame()
{
	int ret = 0;
	CHECK_INIT();

	//put buffer into queue
	if (ret = v4l2_queue_buffer(&vid_dev,videobuf) < 0){
		QV4L2_ERR("failed\n");
		return QV4L2_FAIL;
	}
	videobuf->ishold = QV4L_FALSE;
	return;
}

QV4L2_RET QV4l2Cap_Stop()
{
	int i=0,ret=0;
	CHECK_INIT();

	ret= v4l2_device_off(&vid_dev);
	if (ret < 0){
		QV4L2_ERR("failed\n");
	}
	/* unmap v4l2 mapped buffer */
	for ( i=0;i< BUFFER_CNT ;i++) {
		munmap(vid_dev.vid_buf[i].v4l2_buff,vid_dev.vid_buf[i].v4l2_buff_length);
	}

	close(vid_dev.fd);
	return QV4L2_SUCCESS;
}

QV4L2_RET QV4l2Cap_deinit()
{
	isInited = QV4L_FALSE;
	return QV4L2_SUCCESS;
}
