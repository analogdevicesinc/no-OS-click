/***************************************************************************//**
 *   @file   iio_adxl367.c
 *   @brief  IIO support of the ADXL367 driver.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "no_os_util.h"
#include "iio_adxl367.h"

#define IIO_ADXL367_NB_AXES		3

static const char *const iio_adxl367_axes[IIO_ADXL367_NB_AXES] = {
	"accel_x", "accel_y", "accel_z"
};

static const char *const iio_adxl367_attrs[] = {"raw", "scale", "offset"};

/**
 * @brief Add the three axes, each with raw, scale and offset.
 */
static int iio_adxl367_add_channels(const struct iio_capi_device *iio,
				    struct iio_device *iio_device)
{
	const struct iio_data_format fmt = {0};
	struct iio_channel *chn;
	unsigned int i, j;
	int ret;

	(void)iio;

	for (i = 0; i < IIO_ADXL367_NB_AXES; i++) {
		/* Returns NULL, not an error pointer, when it fails. */
		chn = iio_device_add_channel(iio_device, i, iio_adxl367_axes[i],
					     NULL, NULL, false, false, &fmt);
		if (!chn)
			return -ENOMEM;

		/* The axis, one more, so that none is 0. */
		iio_channel_set_pdata(chn,
				      (struct iio_channel_pdata *)(uintptr_t)(i + 1));

		for (j = 0; j < NO_OS_ARRAY_SIZE(iio_adxl367_attrs); j++) {
			ret = iio_channel_add_attr(chn, iio_adxl367_attrs[j],
						   IIO_ATTR_TYPE_CHANNEL, NULL);
			if (ret)
				return ret;
		}
	}

	return 0;
}

/**
 * @brief Read an axis' raw, scale or offset.
 * @return The length of the value, its NUL included, or a negative error
 *         code.
 */
static int iio_adxl367_read_attr(const struct iio_capi_device *iio,
				 const struct iio_device *iio_device,
				 const struct iio_attr *attr, char *dst, size_t len)
{
	struct adxl367_dev *dev = iio->handle;
	const char *name = iio_attr_get_name(attr);
	int16_t xyz[IIO_ADXL367_NB_AXES];
	uintptr_t axis;
	int ret;

	(void)iio_device;

	if (attr->type != IIO_ATTR_TYPE_CHANNEL)
		return -ENOENT;

	if (!strcmp(name, "raw")) {
		axis = (uintptr_t)iio_channel_get_pdata(attr->iio.chn) - 1;
		if (axis >= IIO_ADXL367_NB_AXES)
			return -EINVAL;
		if (adxl367_get_raw_xyz(dev, &xyz[0], &xyz[1], &xyz[2]))
			return -EIO;
		ret = snprintf(dst, len, "%d", xyz[axis]);
	} else if (!strcmp(name, "scale")) {
		/* ADXL367_ACC_SCALE_FACTOR_MUL billionths at +/-2 g, twice
		 * that for each range above. */
		ret = snprintf(dst, len, "0.%09" PRIu64,
			       (uint64_t)ADXL367_ACC_SCALE_FACTOR_MUL << dev->range);
	} else if (!strcmp(name, "offset")) {
		/* raw is in two's complement, 0 at 0 g. */
		ret = snprintf(dst, len, "0");
	} else {
		return -ENOENT;
	}

	if (ret < 0 || (size_t)ret >= len)
		return -ERANGE;

	return ret + 1;
}

static const struct iio_capi_device_api iio_adxl367_api = {
	.add_channels = iio_adxl367_add_channels,
	.read_attr = iio_adxl367_read_attr,
};

int iio_adxl367_register(struct iio_capi_device *iio,
			 struct adxl367_dev *dev)
{
	if (!iio || !dev)
		return -EINVAL;

	*iio = (struct iio_capi_device) {
		.name = "adxl367",
		.handle = dev,
		.api = &iio_adxl367_api,
	};

	return iio_capi_device_register(iio);
}
