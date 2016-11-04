/******************************************************************************
*
* (c) Copyright 2012-2014 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
*******************************************************************************/

/*****************************************************************************
*
* @file v4l2_helper.c
*
* This file implements the helper functions for v4l2 video devices. 
* 
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a RSP   14/07/14 Initial release
* </pre>
*
* @note
*
******************************************************************************/


#include "v4l2_helper.h"
#include "mediactl_helper.h"


int v4l2_parse_node(char *cardname, char *devnode)
{
	int fd,i=0,ret=0;
	struct v4l2_capability cap;
	char newdev[DEV_NAME_LEN];

	while(1) {
		snprintf (newdev, sizeof (newdev), "/dev/video%u", i);
		if ((fd=open(newdev,O_RDWR))<0)	{
			ret=-ENOENT;
			break;
		}
		if (ioctl(fd,VIDIOC_QUERYCAP,&cap)<0){
			ret=-errno;
			break;
		}
		/* Compare card name */
		if (!strcmp((const char *) cap.card, cardname))	{
			strcpy (devnode, newdev);
			break;
		}
		/* Compare driver name */
		if (!strcmp((const char *) cap.driver, cardname))	{
			strcpy (devnode, newdev);
			break;
		}
		i++;
		close(fd);
	}
	if (ret != -ENOENT)
		close(fd);
	return ret;
}

int v4l2_init(struct v4l2_dev *dev, unsigned int num_buffers)
{
	struct v4l2_capability caps;
	struct v4l2_format fmt;
	struct v4l2_requestbuffers rqbufs;
	char name[5];
	int ret;
	unsigned int i;

	dev->fd = open(dev->devname, O_RDWR);
	if (dev->fd < 0){
		QV4L2_ERR("failed to open %s: %s\n", dev->devname, ERRSTR);
		return -1;
	}

	// TODO caps unused ?
	memset(&caps, 0, sizeof caps);
	ret = ioctl(dev->fd, VIDIOC_QUERYCAP, &caps);
	if (ret < 0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		goto FAILED;
	}

	memset(&fmt, 0, sizeof fmt);
	fmt.type = dev->buf_type;
	ret = ioctl(dev->fd, VIDIOC_G_FMT, &fmt);
	if (ret < 0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		goto FAILED;
	}

	QV4L2_DEG("G_FMT(start): width = %u, height = %u, bytes per line = %u, "
		   "4cc = %.4s, color space = %u\n", fmt.fmt.pix.width,
		   fmt.fmt.pix.height, fmt.fmt.pix.bytesperline,
		   (char*) &fmt.fmt.pix.pixelformat, fmt.fmt.pix.colorspace);

	fmt.fmt.pix = dev->format;
	ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		goto FAILED;
	}
	ret = ioctl(dev->fd, VIDIOC_G_FMT, &fmt);
	if (ret < 0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		goto FAILED;
	}

	QV4L2_DEG("G_FMT(final): width = %u, height = %u, bytes per line = %u, "
		   "4cc = %.4s, color space = %u\n", fmt.fmt.pix.width,
		   fmt.fmt.pix.height, fmt.fmt.pix.bytesperline,
		   (char*)&fmt.fmt.pix.pixelformat, fmt.fmt.pix.colorspace);

	// check if pixel format is supported
	if (fmt.fmt.pix.pixelformat != dev->format.pixelformat) {
		// look up pixel format fourcc code
		for (i = 0; i < 4; ++i) {
			name[i] = fmt.fmt.pix.pixelformat & 0xff;
			fmt.fmt.pix.pixelformat >>= 8;
		}
		name[4] = '\0';

		QV4L2_ERR("Requested pixel format '%s' is not supported by device '%s'\n",
			name, dev->devname);
		goto FAILED;
	}

	// check if resolution is supported
	if (fmt.fmt.pix.width != dev->format.width ||
		fmt.fmt.pix.height != dev->format.height) {
		QV4L2_ERR("Requested resolution '%dx%d' is not supported by device '%s'\n",
				dev->format.width, dev->format.height, dev->devname);
		goto FAILED;
	}

	// check if stride is supported
	if (fmt.fmt.pix.bytesperline != dev->format.bytesperline) {
		QV4L2_ERR("Requested stride '%d' is not supported by device '%s'\n",
				dev->format.bytesperline, dev->devname);
		goto FAILED;
	}

	memset(&rqbufs, 0, sizeof(rqbufs));
	rqbufs.count = num_buffers;
	rqbufs.type = dev->buf_type;
	rqbufs.memory = dev->mem_type;
	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rqbufs);
	if (ret < 0 || rqbufs.count < num_buffers){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		goto FAILED;
	}
	dev->format = fmt.fmt.pix;
	return 0;
	
FAILED:
	close(dev->fd);
	return -1;
}

 int v4l2_queue_buffer(struct v4l2_dev *dev, const struct buffer *buffer)
{
	struct v4l2_buffer buf;
	int ret;

	memset(&buf, 0, sizeof buf);
	buf.type = dev->buf_type;
	buf.index = buffer->index;
	buf.memory = dev->mem_type;
	if(dev->mem_type == V4L2_MEMORY_DMABUF) {
		buf.m.fd = buffer->dbuf_fd;
	}
	ret = ioctl(dev->fd, VIDIOC_QBUF, &buf);
	if (ret <  0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		return -1;
	}
	return 0;
}

 struct buffer *v4l2_dequeue_buffer(struct v4l2_dev *dev, struct buffer *buffers)
{
	struct v4l2_buffer buf;
	int ret;
	memset(&buf, 0, sizeof (buf));
	buf.type = dev->buf_type;
	buf.memory = dev->mem_type;
	ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf);
	if (ret <  0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		return NULL;
	}
	return &buffers[buf.index];
}

/* turn off video device */
 int  v4l2_device_off(struct v4l2_dev *d)
{
	int ret;
	ret = ioctl(d->fd, VIDIOC_STREAMOFF, &d->buf_type);
	return ret;
}

/* turn on video device */
 int  v4l2_device_on(struct v4l2_dev *d)
{
	int ret;
	ret = ioctl(d->fd, VIDIOC_STREAMON, &d->buf_type);
	return ret;
}

int v4l2_mt9m_set_ctrl(int id, int value)
{
	int fd;
	int ret;
	char subdev_name[DEV_NAME_LEN];
	struct v4l2_queryctrl query;
	struct v4l2_control ctrl;

	get_entity_devname(MEDIA_NODE_0, MEDIA_MT9M_ENTITY, subdev_name);

	QV4L2_DEG("subdev_name = %s\n",subdev_name);
	fd = open(subdev_name, O_RDWR);
	if (fd < 0){
		QV4L2_ERR("failed to open %s: %s\n", subdev_name,ERRSTR);
		return -1;
	}
	memset(&query, 0, sizeof(query));
	query.id = id;
	ret = ioctl(fd, VIDIOC_QUERYCTRL, &query);
	if (ret < 0){
		QV4L2_ERR("failed: %s\n", ERRSTR);
		goto FAILED;
	}

	if (query.flags & V4L2_CTRL_FLAG_DISABLED) {
		QV4L2_ERR("V4L2_CTRL_FLAG_DISABLED\n");
	} else {
		memset(&ctrl, 0, sizeof(ctrl));
		ctrl.id = query.id;
		ctrl.value = value;
		ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);
		if (ret < 0){
			QV4L2_ERR("failed: %s\n", ERRSTR);
			goto FAILED;
		}
	}
	
	QV4L2_DEG("ok\n");
	close(fd);
	return 0;
	
FAILED:
	close(fd);
	return -1;
}


void v4l2_mt9m_configure()
{
	v4l2_mt9m_set_ctrl(V4L2_CID_GAIN, 100);
	v4l2_mt9m_set_ctrl(V4L2_CID_VFLIP, 0);
	v4l2_mt9m_set_ctrl(V4L2_CID_EXPOSURE_AUTO, 0);
}

