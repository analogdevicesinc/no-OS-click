/***************************************************************************//**
 *   @file   iio_ad7091r.h
 *   @brief  IIO support of the AD7091R driver.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef IIO_AD7091R_H_
#define IIO_AD7091R_H_

#include "iio_device.h"
#include "ad7091r.h"

/*
 * An AD7091R served to libiio clients through libiio's CAPI port, as the
 * IIO device "ad7091r". It has one channel, voltage0, with raw, scale
 * (millivolts) and offset; each raw read is a new conversion.
 */
struct iio_ad7091r {
	struct iio_capi_device iio;
	struct ad7091r_dev *dev;
	/* The reference, in millivolts: 2500 for the internal one. */
	uint32_t vref_mv;
};

/* Add the AD7091R to the IIO context. iio is filled here and, like dev,
 * kept for as long as the context exists. Call before the context is
 * created (iiod_<transport>_run()). Returns 0 or a negative error code. */
int iio_ad7091r_register(struct iio_ad7091r *iio, struct ad7091r_dev *dev,
			 uint32_t vref_mv);

#endif /* IIO_AD7091R_H_ */
