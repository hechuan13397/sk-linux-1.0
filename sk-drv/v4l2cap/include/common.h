
#ifndef COMMON_H
#define COMMON_H

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
#include <linux/videodev2.h>
#include <mediactl/mediactl.h>
#include <mediactl/v4l2subdev.h>


#define _QV4L2_DEG

#define ERRSTR strerror(errno)

#ifdef _QV4L2_DEG
#define QV4L2_DEG(format,args...)  printf("[DEG]%s-%d-"format,__FUNCTION__,__LINE__,##args);
#else
#define QV4L2_DEG(format,args...)
#endif

#define QV4L2_ERR(format,args...)  printf("[ERR]%s-%d-"format,__FUNCTION__,__LINE__,##args);

#define DEV_NAME_LEN 32

/*  media configuration */
typedef enum
{
	MEDIA_NODE_0,
	MEDIA_NODE_1
} media_node; 

typedef enum
{
	QV4L_FALSE = 0,
	QV4L_TRUE
}QV4L_BOOL;

#define MEDIA_MT9M_ENTITY "mt9m001 0-005d"

#define BUFFER_CNT 3
#define MAX_BUFFER_CNT 5


#endif
