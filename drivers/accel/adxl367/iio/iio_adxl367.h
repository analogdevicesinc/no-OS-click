/***************************************************************************//**
 *   @file   iio_adxl367.h
 *   @brief  IIO support of the ADXL367 driver.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef IIO_ADXL367_H_
#define IIO_ADXL367_H_

#include "iio_device.h"
#include "adxl367.h"

/*
 * An ADXL367 served to libiio clients through libiio's CAPI port, as the
 * IIO device "adxl367". It has three channels, accel_x, accel_y and
 * accel_z, each with raw, scale (m/s^2) and offset.
 */

/* Add the ADXL367 to the IIO context. dev must be measuring; iio is filled
 * here and, like dev, kept for as long as the context exists. Call before
 * the context is created (iiod_<transport>_run()). Returns 0 or a negative
 * error code. */
int iio_adxl367_register(struct iio_capi_device *iio,
			 struct adxl367_dev *dev);

#endif /* IIO_ADXL367_H_ */
