/*
 * Driver for MT9M001 CMOS Image Sensor from Micron
 *
 * Copyright (C) 2008, Guennadi Liakhovetski <kernel@pengutronix.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

 /*
  * changes based on xilinx-tpg.c 
  * 
 
 */

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <media/soc_camera.h>
#include <media/drv-intf/soc_mediabus.h>
#include <media/v4l2-clk.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>
#include <linux/io.h>

//#define IIC_LOOP_TEST
//#define IIC_TURN_OFF
#define WITH_SUBDEV

#define KER_LOG(formt,args...)         printk(KERN_ERR"%s-%d"formt,__FUNCTION__,__LINE__,##args);


/*
 * mt9m001 i2c address 0x5d
 * The platform has to define struct i2c_board_info objects and link to them
 * from struct soc_camera_host_desc
 */

/* mt9m001 selected register addresses */
#define MT9M001_CHIP_VERSION		0x00
#define MT9M001_ROW_START		0x01
#define MT9M001_COLUMN_START		0x02
#define MT9M001_WINDOW_HEIGHT		0x03
#define MT9M001_WINDOW_WIDTH		0x04
#define MT9M001_HORIZONTAL_BLANKING	0x05
#define MT9M001_VERTICAL_BLANKING	0x06
#define MT9M001_OUTPUT_CONTROL		0x07
#define MT9M001_SHUTTER_WIDTH		0x09
#define MT9M001_FRAME_RESTART		0x0b
#define MT9M001_SHUTTER_DELAY		0x0c
#define MT9M001_RESET			0x0d
#define MT9M001_READ_OPTIONS1		0x1e
#define MT9M001_READ_OPTIONS2		0x20
#define MT9M001_GLOBAL_GAIN		0x35
#define MT9M001_CHIP_ENABLE		0xF1

#define MT9M001_MAX_WIDTH		1280
#define MT9M001_MAX_HEIGHT		1024
#define MT9M001_MIN_WIDTH		48
#define MT9M001_MIN_HEIGHT		32
#define MT9M001_COLUMN_SKIP		20
#define MT9M001_ROW_SKIP		12

/* MT9M001 has only one fixed colorspace per pixelcode */
struct mt9m001_datafmt {
	u32	code;
	enum v4l2_colorspace		colorspace;
};

/* Find a data format by a pixel code in an array */
static const struct mt9m001_datafmt *mt9m001_find_datafmt(
	u32 code, const struct mt9m001_datafmt *fmt,
	int n)
{
	int i;
	for (i = 0; i < n; i++)
		if (fmt[i].code == code)
			return fmt + i;

	return NULL;
}

static const struct mt9m001_datafmt mt9m001_colour_fmts[] = {
	/*
	 * Order important: first natively supported,
	 * second supported with a GPIO extender
	 */
	{MEDIA_BUS_FMT_RBG888_1X24, V4L2_COLORSPACE_ADOBERGB},//changed by hechuan
	{MEDIA_BUS_FMT_SBGGR8_1X8, V4L2_COLORSPACE_ADOBERGB},
};

static const struct mt9m001_datafmt mt9m001_monochrome_fmts[] = {
	/* Order important - see above */
	{MEDIA_BUS_FMT_Y10_1X10, V4L2_COLORSPACE_JPEG},
	{MEDIA_BUS_FMT_Y8_1X8, V4L2_COLORSPACE_JPEG},
};

struct mt9m001 {
	struct device *dev;		//add by hechuan
	struct v4l2_subdev subdev;
	struct v4l2_ctrl_handler hdl;
	struct {
		/* exposure/auto-exposure cluster */
		struct v4l2_ctrl *autoexposure;
		struct v4l2_ctrl *exposure;
	};
	struct v4l2_rect rect;	/* Sensor window */
	struct v4l2_clk *clk;
	const struct mt9m001_datafmt *fmt;
	const struct mt9m001_datafmt *fmts;
	int num_fmts;
	unsigned int total_h;
	unsigned short y_skip_top;	/* Lines to skip at the top */

	//add by hechuan
	struct media_pad pads[2];
	unsigned int npads;
	bool has_input;

};

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




//for vdma register write directly
static void vdma_register_write_for_test(void)
{
#define VDMA_BASEADDR_WILD	(pvdma_base_vir)
	char * pvdma_base_vir = ioremap(VDMA_BASEADDR_PHY, 1024*1024);
	writel(0x4,(VDMA_BASEADDR_WILD+0x30));
	writel(0x8,(VDMA_BASEADDR_WILD+0x30));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0xAC));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0xAC+4));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0xAC+8));
	writel(0x00000C80,(VDMA_BASEADDR_WILD+0xA8));
	writel(H_ACTIVE*4,(VDMA_BASEADDR_WILD+0xA4));
	writel(0x3,(VDMA_BASEADDR_WILD+0x30));
	writel(V_ACTIVE,(VDMA_BASEADDR_WILD+0xA0));
	writel(0x4,(VDMA_BASEADDR_WILD+0x00));
	writel(0x8,(VDMA_BASEADDR_WILD+0x00));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0x05c));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0x060));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0x064));
	writel(0x00000C80,(VDMA_BASEADDR_WILD+0x058));
	writel(H_ACTIVE*4,(VDMA_BASEADDR_WILD+0x054));
	writel(0x00000003,(VDMA_BASEADDR_WILD+0x000));
	writel(V_ACTIVE,(VDMA_BASEADDR_WILD+0x050));
	iounmap(pvdma_base_vir);


#define VTC_BASEADDR		(pVTC_base_vir)
	char * pVTC_base_vir = ioremap(VTC_BASEADDR_PHY, 1024*1024);

	
	//vtc config
	writel(((1078<<16)|536),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG1_OFFSET));
	writel(((22<<16)|11),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG2_OFFSET));
	writel(((210<<16)|22),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG3_OFFSET));
	writel(((46<<16)|23),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG4_OFFSET));
	writel(0x04,(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG5_OFFSET));

	iounmap(pVTC_base_vir);
	KER_LOG("set register completed\n");

}

static struct mt9m001 *to_mt9m001(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct mt9m001, subdev);
}

#ifdef IIC_TURN_OFF
static int reg_read(struct i2c_client *client, const u8 reg)
{
	return 0;
}
static int reg_write(struct i2c_client *client, const u8 reg,
		     const u16 data)
{
	return 0;
}
#else
static int reg_read(struct i2c_client *client, const u8 reg)
{
	return i2c_smbus_read_word_swapped(client, reg);
}

static int reg_write(struct i2c_client *client, const u8 reg,
		     const u16 data)
{
	return i2c_smbus_write_word_swapped(client, reg, data);
}
#endif

static int reg_set(struct i2c_client *client, const u8 reg,
		   const u16 data)
{
	int ret;

	ret = reg_read(client, reg);
	if (ret < 0)
		return ret;
	return reg_write(client, reg, ret | data);
}

static int reg_clear(struct i2c_client *client, const u8 reg,
		     const u16 data)
{
	int ret;

	ret = reg_read(client, reg);
	if (ret < 0)
		return ret;
	return reg_write(client, reg, ret & ~data);
}

static int mt9m001_init(struct i2c_client *client)
{
	int ret;

	dev_dbg(&client->dev, "%s\n", __func__);

	/*
	 * We don't know, whether platform provides reset, issue a soft reset
	 * too. This returns all registers to their default values.
	 */
	ret = reg_write(client, MT9M001_RESET, 1);
	if (!ret)
		ret = reg_write(client, MT9M001_RESET, 0);

	/* Disable chip, synchronous option update */
	if (!ret)
		ret = reg_write(client, MT9M001_OUTPUT_CONTROL, 0);

	return ret;
}

static int mt9m001_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	KER_LOG("\n");
	/* Switch to master "normal" mode or stop sensor readout */
	if (reg_write(client, MT9M001_OUTPUT_CONTROL, enable ? 2 : 0) < 0)
		return -EIO;
	return 0;
}

static int mt9m001_s_crop(struct v4l2_subdev *sd, const struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);
	struct v4l2_rect rect = a->c;
	int ret;
	const u16 hblank = 9, vblank = 25;

	if (mt9m001->fmts == mt9m001_colour_fmts)
		/*
		 * Bayer format - even number of rows for simplicity,
		 * but let the user play with the top row.
		 */
		rect.height = ALIGN(rect.height, 2);

	/* Datasheet requirement: see register description */
	rect.width = ALIGN(rect.width, 2);
	rect.left = ALIGN(rect.left, 2);

	soc_camera_limit_side(&rect.left, &rect.width,
		     MT9M001_COLUMN_SKIP, MT9M001_MIN_WIDTH, MT9M001_MAX_WIDTH);

	soc_camera_limit_side(&rect.top, &rect.height,
		     MT9M001_ROW_SKIP, MT9M001_MIN_HEIGHT, MT9M001_MAX_HEIGHT);

	mt9m001->total_h = rect.height + mt9m001->y_skip_top + vblank;

	/* Blanking and start values - default... */
	ret = reg_write(client, MT9M001_HORIZONTAL_BLANKING, hblank);
	if (!ret)
		ret = reg_write(client, MT9M001_VERTICAL_BLANKING, vblank);

	/*
	 * The caller provides a supported format, as verified per
	 * call to .set_fmt(FORMAT_TRY).
	 */
	if (!ret)
		ret = reg_write(client, MT9M001_COLUMN_START, rect.left);
	if (!ret)
		ret = reg_write(client, MT9M001_ROW_START, rect.top);
	if (!ret)
		ret = reg_write(client, MT9M001_WINDOW_WIDTH, rect.width - 1);
	if (!ret)
		ret = reg_write(client, MT9M001_WINDOW_HEIGHT,
				rect.height + mt9m001->y_skip_top - 1);
	if (!ret && v4l2_ctrl_g_ctrl(mt9m001->autoexposure) == V4L2_EXPOSURE_AUTO)
		ret = reg_write(client, MT9M001_SHUTTER_WIDTH, mt9m001->total_h);

	if (!ret)
		mt9m001->rect = rect;

	return ret;
}

static int mt9m001_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);

	a->c	= mt9m001->rect;
	a->type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

static int mt9m001_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	a->bounds.left			= MT9M001_COLUMN_SKIP;
	a->bounds.top			= MT9M001_ROW_SKIP;
	a->bounds.width			= MT9M001_MAX_WIDTH;
	a->bounds.height		= MT9M001_MAX_HEIGHT;
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	return 0;
}

static int mt9m001_get_fmt(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_format *format)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);
	struct v4l2_mbus_framefmt *mf = &format->format;

	if (format->pad)
		return -EINVAL;

	mf->width	= mt9m001->rect.width;
	mf->height	= mt9m001->rect.height;
	mf->code	= mt9m001->fmt->code;
	mf->colorspace	= mt9m001->fmt->colorspace;
	mf->field	= V4L2_FIELD_NONE;
	KER_LOG("\n");
	return 0;
}

static int mt9m001_s_fmt(struct v4l2_subdev *sd,
			 struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);
	struct v4l2_crop a = {
		.c = {
			.left	= mt9m001->rect.left,
			.top	= mt9m001->rect.top,
			.width	= mf->width,
			.height	= mf->height,
		},
	};
	int ret;

	/* No support for scaling so far, just crop. TODO: use skipping */
	ret = mt9m001_s_crop(sd, &a);
	if (!ret) {
		mf->width	= mt9m001->rect.width;
		mf->height	= mt9m001->rect.height;
		KER_LOG("mf->code = [%d]\n",mf->code);
		mt9m001->fmt	= mt9m001_find_datafmt(mf->code,
					mt9m001->fmts, mt9m001->num_fmts);
		mf->colorspace	= mt9m001->fmt->colorspace;
	}
	KER_LOG("\n");
	return ret;
}

static int mt9m001_set_fmt(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_format *format)
{
	struct v4l2_mbus_framefmt *mf = &format->format;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);
	const struct mt9m001_datafmt *fmt;
	
	KER_LOG("\n");

	if (format->pad)
		return -EINVAL;

	v4l_bound_align_image(&mf->width, MT9M001_MIN_WIDTH,
		MT9M001_MAX_WIDTH, 1,
		&mf->height, MT9M001_MIN_HEIGHT + mt9m001->y_skip_top,
		MT9M001_MAX_HEIGHT + mt9m001->y_skip_top, 0, 0);

	if (mt9m001->fmts == mt9m001_colour_fmts)
		mf->height = ALIGN(mf->height - 1, 2);

	fmt = mt9m001_find_datafmt(mf->code, mt9m001->fmts,
				   mt9m001->num_fmts);
	if (!fmt) {
		fmt = mt9m001->fmt;
		mf->code = fmt->code;
	}

	mf->colorspace	= fmt->colorspace;

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE)
		return mt9m001_s_fmt(sd, mf);
	cfg->try_fmt = *mf;
	return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int mt9m001_g_register(struct v4l2_subdev *sd,
			      struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg > 0xff)
		return -EINVAL;

	reg->size = 2;
	reg->val = reg_read(client, reg->reg);

	if (reg->val > 0xffff)
		return -EIO;

	return 0;
}

static int mt9m001_s_register(struct v4l2_subdev *sd,
			      const struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (reg->reg > 0xff)
		return -EINVAL;

	if (reg_write(client, reg->reg, reg->val) < 0)
		return -EIO;

	return 0;
}
#endif

static int mt9m001_s_power(struct v4l2_subdev *sd, int on)
{
	return 0;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct mt9m001 *mt9m001 = to_mt9m001(client);

	return soc_camera_set_power(&client->dev, ssdd, mt9m001->clk, on);
}

static int mt9m001_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct mt9m001 *mt9m001 = container_of(ctrl->handler,
					       struct mt9m001, hdl);
	s32 min, max;

	switch (ctrl->id) {
	case V4L2_CID_EXPOSURE_AUTO:
		min = mt9m001->exposure->minimum;
		max = mt9m001->exposure->maximum;
		mt9m001->exposure->val =
			(524 + (mt9m001->total_h - 1) * (max - min)) / 1048 + min;
		break;
	}
	return 0;
}

static int mt9m001_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct mt9m001 *mt9m001 = container_of(ctrl->handler,
					       struct mt9m001, hdl);
	struct v4l2_subdev *sd = &mt9m001->subdev;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct v4l2_ctrl *exp = mt9m001->exposure;
	int data;
	
	KER_LOG("ctrl->id = 0x%x,ctrl->val = %d\n",ctrl->id,ctrl->val);

	switch (ctrl->id) {
	case V4L2_CID_VFLIP:
		if (ctrl->val)
			data = reg_set(client, MT9M001_READ_OPTIONS2, 0x8000);
		else
			data = reg_clear(client, MT9M001_READ_OPTIONS2, 0x8000);
		if (data < 0)
			return -EIO;
		return 0;

	case V4L2_CID_GAIN:
		/* See Datasheet Table 7, Gain settings. */
		if (ctrl->val <= ctrl->default_value) {
			/* Pack it into 0..1 step 0.125, register values 0..8 */
			unsigned long range = ctrl->default_value - ctrl->minimum;
			data = ((ctrl->val - (s32)ctrl->minimum) * 8 + range / 2) / range;

			dev_dbg(&client->dev, "Setting gain %d\n", data);
			data = reg_write(client, MT9M001_GLOBAL_GAIN, data);
			if (data < 0)
				return -EIO;
		} else {
			/* Pack it into 1.125..15 variable step, register values 9..67 */
			/* We assume qctrl->maximum - qctrl->default_value - 1 > 0 */
			unsigned long range = ctrl->maximum - ctrl->default_value - 1;
			unsigned long gain = ((ctrl->val - (s32)ctrl->default_value - 1) *
					       111 + range / 2) / range + 9;

			if (gain <= 32)
				data = gain;
			else if (gain <= 64)
				data = ((gain - 32) * 16 + 16) / 32 + 80;
			else
				data = ((gain - 64) * 7 + 28) / 56 + 96;

			dev_dbg(&client->dev, "Setting gain from %d to %d\n",
				 reg_read(client, MT9M001_GLOBAL_GAIN), data);
			data = reg_write(client, MT9M001_GLOBAL_GAIN, data);
			if (data < 0)
				return -EIO;
		}
		return 0;

	case V4L2_CID_EXPOSURE_AUTO:
		if (ctrl->val == V4L2_EXPOSURE_MANUAL) {
			unsigned long range = exp->maximum - exp->minimum;
			unsigned long shutter = ((exp->val - (s32)exp->minimum) * 1048 +
						 range / 2) / range + 1;

			dev_dbg(&client->dev,
				"Setting shutter width from %d to %lu\n",
				reg_read(client, MT9M001_SHUTTER_WIDTH), shutter);
			if (reg_write(client, MT9M001_SHUTTER_WIDTH, shutter) < 0)
				return -EIO;
		} else {
			const u16 vblank = 25;

			mt9m001->total_h = mt9m001->rect.height +
				mt9m001->y_skip_top + vblank;
			if (reg_write(client, MT9M001_SHUTTER_WIDTH, mt9m001->total_h) < 0)
				return -EIO;
		}
		return 0;
	}
	return -EINVAL;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int mt9m001_video_probe(struct soc_camera_subdev_desc *ssdd,
			       struct i2c_client *client)
{
	struct mt9m001 *mt9m001 = to_mt9m001(client);
	s32 data;
	unsigned long flags;
	int ret;
	int i = 0;

	ret = mt9m001_s_power(&mt9m001->subdev, 1);
	if (ret < 0)
		return ret;

#ifdef IIC_LOOP_TEST
	for (;i < 10000;i++){
		data = reg_read(client, MT9M001_CHIP_VERSION);
		KER_LOG("read data = 0x%x\n",data);
		KER_LOG("read data = %d\n",data);

		mdelay(500);
	} 
	
#endif	

	/* Enable the chip */
	data = reg_write(client, MT9M001_CHIP_ENABLE, 1);
	dev_dbg(&client->dev, "write: %d\n", data);

	/* Read out the chip version register */
	data = reg_read(client, MT9M001_CHIP_VERSION);

	//data read out is really 8431,but we expect it was 8421.
	//hechuan changed
	data = 0x8421;

	/* must be 0x8411 or 0x8421 for colour sensor and 8431 for bw */
	switch (data) {
	case 0x8411:
	case 0x8421:
		mt9m001->fmts = mt9m001_colour_fmts;
		break;
	case 0x8431:
		mt9m001->fmts = mt9m001_monochrome_fmts;
		break;
	default:
		dev_err(&client->dev,
			"No MT9M001 chip detected, register read %x\n", data);
		ret = -ENODEV;
		goto done;
	}

	mt9m001->num_fmts = 0;

	/*
	 * This is a 10bit sensor, so by default we only allow 10bit.
	 * The platform may support different bus widths due to
	 * different routing of the data lines.
	 */
#if 0	 
	if (ssdd->query_bus_param)
		flags = ssdd->query_bus_param(ssdd);
	else
		flags = SOCAM_DATAWIDTH_10;
#endif
	flags = SOCAM_DATAWIDTH_10;

	if (flags & SOCAM_DATAWIDTH_10)
		mt9m001->num_fmts++;
	else
		mt9m001->fmts++;

	if (flags & SOCAM_DATAWIDTH_8)
		mt9m001->num_fmts++;

	mt9m001->fmt = &mt9m001->fmts[0];

	dev_info(&client->dev, "Detected a MT9M001 chip ID %x (%s)\n", data,
		 data == 0x8431 ? "C12STM" : "C12ST");

	ret = mt9m001_init(client);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to initialise the camera\n");
		goto done;
	}

	/* mt9m001_init() has reset the chip, returning registers to defaults */
	ret = v4l2_ctrl_handler_setup(&mt9m001->hdl);

done:
	mt9m001_s_power(&mt9m001->subdev, 0);
	KER_LOG("ret = %d\n",ret);
	return ret;
}

static void mt9m001_video_remove(struct soc_camera_subdev_desc *ssdd)
{
	if (ssdd->free_bus)
		ssdd->free_bus(ssdd);
}

static int mt9m001_g_skip_top_lines(struct v4l2_subdev *sd, u32 *lines)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);

	*lines = mt9m001->y_skip_top;

	return 0;
}

/* -----------------------------------------------------------------------------
 * Media Operations
 */
static const struct media_entity_operations mt9m001_media_ops = {
	.link_validate = v4l2_subdev_link_validate,
};


static const struct v4l2_ctrl_ops mt9m001_ctrl_ops = {
	.g_volatile_ctrl = mt9m001_g_volatile_ctrl,
	.s_ctrl = mt9m001_s_ctrl,
};

static struct v4l2_subdev_core_ops mt9m001_subdev_core_ops = {
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register	= mt9m001_g_register,
	.s_register	= mt9m001_s_register,
#endif
	.s_power	= mt9m001_s_power,
};

static int mt9m001_enum_mbus_code(struct v4l2_subdev *sd,
		struct v4l2_subdev_pad_config *cfg,
		struct v4l2_subdev_mbus_code_enum *code)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct mt9m001 *mt9m001 = to_mt9m001(client);

	if (code->pad || code->index >= mt9m001->num_fmts)
		return -EINVAL;

	code->code = mt9m001->fmts[code->index].code;
	return 0;
}

static int mt9m001_enum_frame_size(struct v4l2_subdev *subdev,
				struct v4l2_subdev_pad_config *cfg,
				struct v4l2_subdev_frame_size_enum *fse)
{

	KER_LOG("\n");
	return 0;
}


static int mt9m001_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);

	/* MT9M001 has all capture_format parameters fixed */
	cfg->flags = V4L2_MBUS_PCLK_SAMPLE_FALLING |
		V4L2_MBUS_HSYNC_ACTIVE_HIGH | V4L2_MBUS_VSYNC_ACTIVE_HIGH |
		V4L2_MBUS_DATA_ACTIVE_HIGH | V4L2_MBUS_MASTER;
	cfg->type = V4L2_MBUS_PARALLEL;
	cfg->flags = soc_camera_apply_board_flags(ssdd, cfg);

	return 0;
}

static int mt9m001_s_mbus_config(struct v4l2_subdev *sd,
				const struct v4l2_mbus_config *cfg)
{
	const struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	struct mt9m001 *mt9m001 = to_mt9m001(client);
	unsigned int bps = soc_mbus_get_fmtdesc(mt9m001->fmt->code)->bits_per_sample;

	if (ssdd->set_bus_param)
		return ssdd->set_bus_param(ssdd, 1 << (bps - 1));

	/*
	 * Without board specific bus width settings we only support the
	 * sensors native bus width
	 */
	return bps == 10 ? 0 : -EINVAL;
}

static int mt9m001_open(struct v4l2_subdev *subdev, struct v4l2_subdev_fh *fh)
{
	KER_LOG("\n");
	return 0;
}

static int mt9m001_close(struct v4l2_subdev *subdev, struct v4l2_subdev_fh *fh)
{
	KER_LOG("\n");
	return 0;
}


static struct v4l2_subdev_video_ops mt9m001_subdev_video_ops = {
	.s_stream	= mt9m001_s_stream,
	.s_crop		= mt9m001_s_crop,
	.g_crop		= mt9m001_g_crop,
	.cropcap	= mt9m001_cropcap,
	.g_mbus_config	= mt9m001_g_mbus_config,
	.s_mbus_config	= mt9m001_s_mbus_config,
};

static const struct v4l2_subdev_sensor_ops mt9m001_subdev_sensor_ops = {
	.g_skip_top_lines	= mt9m001_g_skip_top_lines,
};

static const struct v4l2_subdev_pad_ops mt9m001_subdev_pad_ops = {
	.enum_mbus_code = mt9m001_enum_mbus_code,
	.enum_frame_size	= mt9m001_enum_frame_size,

	.get_fmt	= mt9m001_get_fmt,
	.set_fmt	= mt9m001_set_fmt,
};

static struct v4l2_subdev_ops mt9m001_subdev_ops = {
	.core	= &mt9m001_subdev_core_ops,
	.video	= &mt9m001_subdev_video_ops,
	.sensor	= &mt9m001_subdev_sensor_ops,
	.pad	= &mt9m001_subdev_pad_ops,
};

static const struct v4l2_subdev_internal_ops mt9m001_internal_ops = {
	.open	= mt9m001_open,
	.close	= mt9m001_close,
};


static int mt9m_parse_of(struct mt9m001 *pmt9m)
{

	struct device *dev = pmt9m->dev;
	struct device_node *node = pmt9m->dev->of_node;
	struct device_node *ports;
	struct device_node *port;
	unsigned int nports = 0;
	bool has_endpoint = false;

	ports = of_get_child_by_name(node, "ports");
	if (ports == NULL)
		ports = node;

	for_each_child_of_node(ports, port) {
		const struct xvip_video_format *format;
		struct device_node *endpoint;

		if (!port->name || of_node_cmp(port->name, "port"))
			continue;

		if (nports == 0) {
			endpoint = of_get_next_child(port, NULL);
			if (endpoint)
				has_endpoint = true;
			of_node_put(endpoint);
		}

		/* Count the number of ports. */
		nports++;
	}

	if (nports != 1 && nports != 2) {
		dev_err(dev, "invalid number of ports %u\n", nports);
		return -EINVAL;
	}

	//KER_LOG("nports = [%d] has_endpoint = [%d]\n",nports,has_endpoint);
	pmt9m->npads = nports;
	if (nports == 2 && has_endpoint)
		pmt9m->has_input = true;

	return 0;
}


#ifdef WITH_SUBDEV
static int mt9m001_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	struct v4l2_subdev *subdev;
	struct mt9m001 *mt9m001;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct soc_camera_subdev_desc *ssdd = soc_camera_i2c_to_desc(client);
	int ret;

	//vdma_register_write_for_test();	
#if 0
	if (!ssdd) {
		dev_err(&client->dev, "MT9M001 driver needs platform data\n");
		return -EINVAL;
	}
#endif

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_warn(&adapter->dev,
			 "I2C-Adapter doesn't support I2C_FUNC_SMBUS_WORD\n");
		return -EIO;
	}

	mt9m001 = devm_kzalloc(&client->dev, sizeof(struct mt9m001), GFP_KERNEL);
	if (!mt9m001)
		return -ENOMEM;
	
	mt9m001->dev = &client->dev;
	mt9m_parse_of(mt9m001);

	/* Initialize V4L2 subdevice and media entity */
	subdev = &mt9m001->subdev;
	subdev->internal_ops = &mt9m001_internal_ops;

	/* Initialize V4L2 subdevice and media entity. Pad numbers depend on the
	 * number of pads.
	 */
	if (mt9m001->npads == 2) {
		mt9m001->pads[0].flags = MEDIA_PAD_FL_SINK;
		mt9m001->pads[1].flags = MEDIA_PAD_FL_SOURCE;
	} else {
		mt9m001->pads[0].flags = MEDIA_PAD_FL_SOURCE;
	}

	v4l2_i2c_subdev_init(&mt9m001->subdev, client, &mt9m001_subdev_ops);

	subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	subdev->entity.ops = &mt9m001_media_ops;

	ret = media_entity_pads_init(&subdev->entity, mt9m001->npads, mt9m001->pads);
	if (ret < 0)
		goto error;

	v4l2_ctrl_handler_init(&mt9m001->hdl, 4);
	v4l2_ctrl_new_std(&mt9m001->hdl, &mt9m001_ctrl_ops,
			V4L2_CID_VFLIP, 0, 1, 1, 0);
	v4l2_ctrl_new_std(&mt9m001->hdl, &mt9m001_ctrl_ops,
			V4L2_CID_GAIN, 0, 127, 1, 64);
	mt9m001->exposure = v4l2_ctrl_new_std(&mt9m001->hdl, &mt9m001_ctrl_ops,
			V4L2_CID_EXPOSURE, 1, 255, 1, 255);
	/*
	 * Simulated autoexposure. If enabled, we calculate shutter width
	 * ourselves in the driver based on vertical blanking and frame width
	 */
	mt9m001->autoexposure = v4l2_ctrl_new_std_menu(&mt9m001->hdl,
			&mt9m001_ctrl_ops, V4L2_CID_EXPOSURE_AUTO, 1, 0,
			V4L2_EXPOSURE_AUTO);
	mt9m001->subdev.ctrl_handler = &mt9m001->hdl;
	if (mt9m001->hdl.error)
		return mt9m001->hdl.error;

	v4l2_ctrl_auto_cluster(2, &mt9m001->autoexposure,
					V4L2_EXPOSURE_MANUAL, true);

	/* Second stage probe - when a capture adapter is there */
	mt9m001->y_skip_top	= 0;
	mt9m001->rect.left	= MT9M001_COLUMN_SKIP;
	mt9m001->rect.top	= MT9M001_ROW_SKIP;
	mt9m001->rect.width	= MT9M001_MAX_WIDTH;
	mt9m001->rect.height	= MT9M001_MAX_HEIGHT;

#if 0
	mt9m001->clk = v4l2_clk_get(&client->dev, "mclk");
	if (IS_ERR(mt9m001->clk)) {
		ret = PTR_ERR(mt9m001->clk);
		goto eclkget;
	}
#endif

	mt9m001->subdev.dev = &client->dev;
	ret = v4l2_async_register_subdev(&mt9m001->subdev);
	if (ret < 0){
		KER_LOG("failed\n");
	}

	ret = mt9m001_video_probe(ssdd, client);

	if (ret) {
		v4l2_async_unregister_subdev(&mt9m001->subdev);
error:
		v4l2_ctrl_handler_free(&mt9m001->hdl);
		media_entity_cleanup(&subdev->entity);
	}

	KER_LOG("successed ret = %d\n",ret);
	return ret;
}

static int mt9m001_remove(struct i2c_client *client)
{

	struct mt9m001 *mt9m001 = to_mt9m001(client);
	v4l2_async_unregister_subdev(&mt9m001->subdev);
	v4l2_ctrl_handler_free(&mt9m001->hdl);
	KER_LOG("over\n");

	return 0;
}

#else
static int mt9m001_probe(struct i2c_client *client,
			 const struct i2c_device_id *did)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	int ret;
	s32 data;
	int i = 0;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA)) {
		dev_warn(&adapter->dev,
			 "I2C-Adapter doesn't support I2C_FUNC_SMBUS_WORD\n");
		return -EIO;
	}

	KER_LOG("client->addr = %d\n",client->addr);
		/* Enable the chip */
	data = reg_write(client, MT9M001_CHIP_ENABLE, 1);
	dev_dbg(&client->dev, "write: %d\n", data);
	
	/* Read out the chip version register */
	for (i = 0; i < 100; i++){
		data = reg_read(client, MT9M001_CHIP_VERSION);
		if (data < 0){
			KER_LOG("i = %d failed reg_write = [0x%08x]\n",i,data);
		}else{
			KER_LOG("i = %d successed reg_write = [0x%08x]\n",i,data);
		}	
		msleep(200);
	}
	KER_LOG("over\n");
	return 0;

}
static int mt9m001_remove(struct i2c_client *client)
{
		KER_LOG("over\n");
		return 0;
}
#endif

static const struct i2c_device_id mt9m001_id[] = {
	{ "mt9m001", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, mt9m001_id);

static struct i2c_driver mt9m001_i2c_driver = {
	.driver = {
		.name = "mt9m001",
	},
	.probe		= mt9m001_probe,
	.remove		= mt9m001_remove,
	.id_table	= mt9m001_id,
};

module_i2c_driver(mt9m001_i2c_driver);

MODULE_DESCRIPTION("Micron MT9M001 Camera driver");
MODULE_AUTHOR("Guennadi Liakhovetski <kernel@pengutronix.de>");
MODULE_LICENSE("GPL");
