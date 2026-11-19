/***************************************************************************//**
 *   @file   iio_adt7420.c
 *   @brief  IIO support of the ADT7420 driver.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "no_os_util.h"
#include "iio_adt7420.h"

/* One code in milli degrees Celsius: 1/128 C in 16-bit mode and, with the
 * three flag bits shifted out, 1/16 C in 13-bit mode. */
#define IIO_ADT7420_SCALE_16BIT		"7.8125"
#define IIO_ADT7420_SCALE_13BIT		"62.5"
#define IIO_ADT7420_FLAG_BITS		3

static const char *const iio_adt7420_attrs[] = {"raw", "scale", "offset"};

/**
 * @brief Add the temp channel, with raw, scale and offset.
 */
static int iio_adt7420_add_channels(const struct iio_capi_device *iio,
				    struct iio_device *iio_device)
{
	const struct iio_data_format fmt = {0};
	struct iio_channel *chn;
	size_t i;
	int ret;

	(void)iio;

	/* Returns NULL, not an error pointer, when it fails. */
	chn = iio_device_add_channel(iio_device, 0, "temp", NULL, NULL, false,
				     false, &fmt);
	if (!chn)
		return -ENOMEM;

	for (i = 0; i < NO_OS_ARRAY_SIZE(iio_adt7420_attrs); i++) {
		ret = iio_channel_add_attr(chn, iio_adt7420_attrs[i],
					   IIO_ATTR_TYPE_CHANNEL, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * @brief Read raw, the last conversion result, scale or offset.
 * @return The length of the value, its NUL included, or a negative error
 *         code.
 */
static int iio_adt7420_read_attr(const struct iio_capi_device *iio,
				 const struct iio_device *iio_device,
				 const struct iio_attr *attr, char *dst, size_t len)
{
	struct adt7420_dev *dev = iio->handle;
	const char *name = iio_attr_get_name(attr);
	uint16_t reg;
	int16_t code;
	int ret;

	(void)iio_device;

	if (!strcmp(name, "raw")) {
		/* The temperature, MSB first, then in 13-bit mode three flag
		 * bits. */
		if (adt7420_reg_read(dev, ADT7420_REG_TEMP_MSB, &reg))
			return -EIO;
		code = (int16_t)reg;
		if (!dev->resolution_setting)
			code >>= IIO_ADT7420_FLAG_BITS;
		ret = snprintf(dst, len, "%d", code);
	} else if (!strcmp(name, "scale")) {
		ret = snprintf(dst, len, "%s", dev->resolution_setting ?
			       IIO_ADT7420_SCALE_16BIT : IIO_ADT7420_SCALE_13BIT);
	} else if (!strcmp(name, "offset")) {
		/* raw is in two's complement, 0 at 0 C. */
		ret = snprintf(dst, len, "0");
	} else {
		return -ENOENT;
	}

	if (ret < 0 || (size_t)ret >= len)
		return -ERANGE;

	return ret + 1;
}

static const struct iio_capi_device_api iio_adt7420_api = {
	.add_channels = iio_adt7420_add_channels,
	.read_attr = iio_adt7420_read_attr,
};

int iio_adt7420_register(struct iio_capi_device *iio,
			 struct adt7420_dev *dev)
{
	if (!iio || !dev)
		return -EINVAL;

	*iio = (struct iio_capi_device) {
		.name = "adt7420",
		.handle = dev,
		.api = &iio_adt7420_api,
	};

	return iio_capi_device_register(iio);
}
