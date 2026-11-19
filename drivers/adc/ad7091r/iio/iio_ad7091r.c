/***************************************************************************//**
 *   @file   iio_ad7091r.c
 *   @brief  IIO support of the AD7091R driver.
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
#include "iio_ad7091r.h"

/* 12-bit straight binary: 0 at 0 V, 4095 at the reference less one code. */
#define IIO_AD7091R_RESOLUTION		12

static const char *const iio_ad7091r_attrs[] = {"raw", "scale", "offset"};

/**
 * @brief Add the voltage0 channel, with raw, scale and offset.
 */
static int iio_ad7091r_add_channels(const struct iio_capi_device *iio,
				    struct iio_device *iio_device)
{
	const struct iio_data_format fmt = {0};
	struct iio_channel *chn;
	size_t i;
	int ret;

	(void)iio;

	/* Returns NULL, not an error pointer, when it fails. */
	chn = iio_device_add_channel(iio_device, 0, "voltage0", NULL, NULL,
				     false, false, &fmt);
	if (!chn)
		return -ENOMEM;

	for (i = 0; i < NO_OS_ARRAY_SIZE(iio_ad7091r_attrs); i++) {
		ret = iio_channel_add_attr(chn, iio_ad7091r_attrs[i],
					   IIO_ATTR_TYPE_CHANNEL, NULL);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * @brief Read raw, a new conversion, scale or offset.
 * @return The length of the value, its NUL included, or a negative error
 *         code.
 */
static int iio_ad7091r_read_attr(const struct iio_capi_device *iio,
				 const struct iio_device *iio_device,
				 const struct iio_attr *attr, char *dst, size_t len)
{
	const struct iio_ad7091r *adc = (const struct iio_ad7091r *)iio;
	const char *name = iio_attr_get_name(attr);
	uint64_t scale;
	uint16_t code;
	int ret;

	(void)iio_device;

	if (!strcmp(name, "raw")) {
		if (ad7091r_read_sample(adc->dev, &code))
			return -EIO;
		ret = snprintf(dst, len, "%u", code);
	} else if (!strcmp(name, "scale")) {
		/* vref_mv / 2^12, rounded to nine decimals. */
		scale = ((uint64_t)adc->vref_mv * 1000000000ULL +
			 (1U << (IIO_AD7091R_RESOLUTION - 1))) >>
			IIO_AD7091R_RESOLUTION;
		ret = snprintf(dst, len, "%" PRIu64 ".%09" PRIu64,
			       scale / 1000000000, scale % 1000000000);
	} else if (!strcmp(name, "offset")) {
		/* Straight binary from 0 V. */
		ret = snprintf(dst, len, "0");
	} else {
		return -ENOENT;
	}

	if (ret < 0 || (size_t)ret >= len)
		return -ERANGE;

	return ret + 1;
}

static const struct iio_capi_device_api iio_ad7091r_api = {
	.add_channels = iio_ad7091r_add_channels,
	.read_attr = iio_ad7091r_read_attr,
};

int iio_ad7091r_register(struct iio_ad7091r *iio, struct ad7091r_dev *dev,
			 uint32_t vref_mv)
{
	if (!iio || !dev || !vref_mv)
		return -EINVAL;

	*iio = (struct iio_ad7091r) {
		.iio = {
			.name = "ad7091r",
			.handle = dev,
			.api = &iio_ad7091r_api,
		},
		.dev = dev,
		.vref_mv = vref_mv,
	};

	return iio_capi_device_register(&iio->iio);
}
