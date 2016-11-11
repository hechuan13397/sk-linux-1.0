

#ifndef QT_V4L2_CAP_H
#define QT_V4L2_CAP_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	QV4L2_CAP_IMG_RGBA = 0,
}QV4L2_CAP_IMG_FORMAT;

typedef enum {
	QV4L2_SUCCESS = 0,
	QV4L2_FAIL = -1,
	QV4L2_NINIT = -2,
	QV4L2_NSTART = -3,
	QV4L2_INVALID = -4,
}QV4L2_RET;

typedef  struct {
	unsigned char *frame_buff;
	unsigned int buff_length;
}strFrameBuf;

#define CHECK_INIT()	\
	do {	\
		if (!isInited){		\
			printf("module not inited\n");		\
			return QV4L2_NINIT; \
		} \
	}while(0);

/*
	qt v4l2 capture init
*/
QV4L2_RET QV4l2Cap_init();

/*
	qt v4l2 set image capture width and height
	
*/
void QV4l2Cap_SetWH(int w,int h);

/*
	qt v4l2 set image format
	
*/
void QV4l2Cap_SetImgFormat(QV4L2_CAP_IMG_FORMAT format);

/*
	start to capture camera image
*/
QV4L2_RET QV4l2Cap_Start();

/*
	get one frame for processing.
	such as displaying. or opencv processing
	it must be released by QV4l2Cap_FrameRelease(). before obtain frame
*/
QV4L2_RET QV4l2Cap_ObtainFrame(strFrameBuf *buf);

/*
	release a frame after using.
	
*/
QV4L2_RET QV4l2Cap_ReleaseFrame();

/*
	stop to capture camera image
*/
QV4L2_RET QV4l2Cap_Stop();


/*
	qt v4l2 capture deinit
*/
QV4L2_RET QV4l2Cap_deinit();

#ifdef __cplusplus
}
#endif

#endif

