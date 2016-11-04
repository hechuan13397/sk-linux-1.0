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
* @file mediactl_helper.c
*
* This file implements the  helper functions for media library.
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


#include "common.h"
#include "mediactl_helper.h"
#include "v4l2_helper.h"

#define MEDIA_MT9M_FMT "\"mt9m001 0-005d\":%d [fmt:UYVY/%dx%d]"


/*Print media device details */ 
void print_media_info(const struct media_device_info *info)
{
	printf("Media controller API version %u.%u.%u\n\n",
						   (info->media_version << 16) & 0xff,
						   (info->media_version << 8) & 0xff,
						   (info->media_version << 0) & 0xff);
	printf("Media device information\n"
						   "------------------------\n"
						   "driver			%s\n"
						   "model			%s\n"
						   "serial			%s\n"
						   "bus info		%s\n"
						   "hw revision 	0x%x\n"
						   "driver version	%u.%u.%u\n\n",
						   info->driver, info->model,
						   info->serial, info->bus_info,
						   info->hw_revision,
						   (info->driver_version << 16) & 0xff,
						   (info->driver_version << 8) & 0xff,
						   (info->driver_version << 0) & 0xff);
}

int set_media_control(struct v4l2_dev* dev,media_node node)
{
	struct media_device *media;
	char media_devname[DEV_NAME_LEN];
	char media_formats[100];
	int ret = -1;

	/* TODO: obtain media node based on model instead of number */
	sprintf(media_devname, "/dev/media%d", node);
	/* Set media device node */
	media = media_device_new(media_devname);
	if (!media){
		QV4L2_ERR( "Failed to create media device %s\n", media_devname);
		return -1;
	}
	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	if (ret < 0){
		QV4L2_ERR("Failed to create media device %s\n", media_devname);
		goto FAILED;
	}

#ifdef DEBUG_MODE
	const struct media_device_info *info = media_get_info(media);
	print_media_info(info);
#endif

	memset(media_formats, 0, sizeof (media_formats));
	if (node == MEDIA_NODE_0) {
		/* Set TPG input resolution */
		sprintf(media_formats, MEDIA_MT9M_FMT, 0, dev->format.width, dev->format.height);
		ret = v4l2_subdev_parse_setup_formats(media, media_formats);
		if (0 != ret){
			QV4L2_ERR("Unable to setup formats: %s (%d)\n", strerror(-ret), -ret);
			goto FAILED;
		}
	}
	
	media_device_unref(media);
	return 0;

FAILED:
	if (media)
		media_device_unref(media);
	return -1;
}

/* Helper function to search for an entity with a name equal to @name*/
int get_entity_state(media_node node, char *name)
{
	struct media_entity *entity;
	struct media_device *media;
	int status = 0, ret = 0;
	char dev_name[DEV_NAME_LEN];

	/* Set media device node */
	sprintf(dev_name, "/dev/media%d", node);
	media = media_device_new(dev_name);
	if (!media){
		QV4L2_ERR("Failed to create media device device\n");
		return -1;
	}

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	if (ret < 0){
		QV4L2_ERR("Failed to create media device device\n");
		media_device_unref(media);
		return -1;
	}
	
	/* Check if entity is present */
	entity = media_get_entity_by_name(media, name, strlen(name));
	if (entity)
		status = 1;

	media_device_unref(media);
	return status;
}

/* Helper function that returns the full path and name to the device node 
 * corresponding to the given entity i.e (/dev/v4l-subdev* )  */
int get_entity_devname(media_node node, char *name, char *subdev_name)
{
	struct media_device *media;
	struct media_entity *entity;
	char dev_name[DEV_NAME_LEN];
	int ret;

	/* Set media device node */
	sprintf(dev_name, "/dev/media%d", node);
	media = media_device_new(dev_name);
	if (!media){
		QV4L2_ERR("Failed to create media device\n");
		return -1;
	}

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	if (ret < 0){
		QV4L2_ERR("Failed to enumerate '%s'\n", dev_name);
		goto FAILED;
	}
	entity = media_get_entity_by_name(media, name, strlen(name));
	if (!entity){
		QV4L2_ERR("Entity '%s' not found\n", name);
		goto FAILED;
	}

	strcpy(subdev_name, media_entity_get_devname(entity));
	if (!subdev_name[0])	{
		QV4L2_ERR("failed\n");
		goto FAILED;
	}	
	media_device_unref(media);
	return 0;
	
FAILED:
	media_device_unref(media);
	return -1;
}

/* Helper function that returns the total number of links that originate
    from or arrive at the the media entity*/
int get_entity_link_count(media_node node, char *name)
{
	struct media_entity *entity;
	struct media_device *media;
	unsigned int link_count = 0;
	int ret = 0;
	char dev_name[DEV_NAME_LEN];

	/* Set media device node */
	sprintf(dev_name, "/dev/media%d", node);
	media = media_device_new(dev_name);
	if (!media){
		QV4L2_ERR("fail\n");
		return -1;
	}

	/* Enumerate entities, pads and links */
	ret = media_device_enumerate(media);
	if (ret < 0){
		QV4L2_ERR("fail\n");
		media_device_unref(media);
		return -1;
	}
	
	/* Get entity link count */
	entity = media_get_entity_by_name(media, name, strlen(name));
	if (entity)
		link_count = media_entity_get_links_count(entity);

	media_device_unref(media);
	return link_count;
}

