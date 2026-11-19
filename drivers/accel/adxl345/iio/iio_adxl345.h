/***************************************************************************//**
 *   @file   iio_adxl345.h
 *   @brief  IIO support of the ADXL345 driver.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef IIO_ADXL345_H_
#define IIO_ADXL345_H_

#include "iio_device.h"
#include "adxl345.h"

/*
 * An ADXL345 served to libiio clients through libiio's CAPI port, as the
 * IIO device "adxl345" (ADXL345, ADXL346). It has three channels, accel_x,
 * accel_y and accel_z, each with raw, scale (m/s^2) and offset.
 */

/* Add the ADXL345 to the IIO context. dev must be measuring; iio is filled
 * here and, like dev, kept for as long as the context exists. Call before
 * the context is created (iiod_<transport>_run()). Returns 0 or a negative
 * error code. */
int iio_adxl345_register(struct iio_capi_device *iio,
			 struct adxl345_dev *dev);

#endif /* IIO_ADXL345_H_ */
