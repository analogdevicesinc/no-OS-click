/***************************************************************************//**
 *   @file   iio_adt7420.h
 *   @brief  IIO support of the ADT7420 driver.
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*******************************************************************************/
#ifndef IIO_ADT7420_H_
#define IIO_ADT7420_H_

#include "iio_device.h"
#include "adt7420.h"

/*
 * An ADT7420 served to libiio clients through libiio's CAPI port, as the
 * IIO device "adt7420" (I2C parts: ADT7420, ADT7410, ADT7422). It has one
 * channel, temp, with raw, scale (milli degrees Celsius) and offset.
 */

/* Add the ADT7420 to the IIO context. iio is filled here and, like dev,
 * kept for as long as the context exists. Call before the context is
 * created (iiod_<transport>_run()). Returns 0 or a negative error code. */
int iio_adt7420_register(struct iio_capi_device *iio,
			 struct adt7420_dev *dev);

#endif /* IIO_ADT7420_H_ */
