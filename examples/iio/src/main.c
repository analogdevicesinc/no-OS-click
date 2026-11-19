/*
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***************************************************************************//**
 *   @file   main.c
 *   @brief  Click board IIO application for CAPI.
 *
 *   Serve the parts found on the Click socket to libiio clients, on the
 *   console UART (URI serial:<port>,115200): an ADT7420 (Surface Temp 2
 *   Click) on I2C as the IIO device "adt7420", an ADXL345 (Accel Click) on
 *   I2C or SPI as "adxl345", an ADXL367 (Accel 22 Click) on I2C or SPI as
 *   "adxl367", an AD7091R (ISO ADC 2 Click) on SPI as "ad7091r".
*******************************************************************************/
#include "capi_platform.h"
#include "capi_uart.h"
#include "capi_i2c.h"
#include "capi_spi.h"
#include "capi_gpio.h"
#include "parameters.h"
#include "iiod_uart.h"
#include "ad7091r.h"
#include "adt7420.h"
#include "adxl345.h"
#include "adxl367.h"
#include "iio_ad7091r.h"
#include "iio_adt7420.h"
#include "iio_adxl345.h"
#include "iio_adxl367.h"
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ADT7420_CLICK_I2C_ADDR	ADT7420_ADDRESS(0, 0)
/* The AD7091R internal reference, in millivolts. */
#define AD7091R_CLICK_VREF_MV	2500

/* The Accel Click's and the Accel 22 Click's ADDR SEL jumpers put them at
 * either. */
static const uint8_t accel_click_i2c_addr[] = { 0x1D, 0x53 };

/* Where an accelerometer was found, for the next one not to be looked for
 * there. */
struct accel_click_taken {
	/* 0 for none. */
	uint8_t i2c_addr;
	bool spi;
};

/**
 * @brief Bring up the ADT7420 on the Click socket and add it to the IIO
 *        context.
 * @param i2c - The Click socket I2C controller.
 * @return 0 in case of success, negative error code otherwise.
 */
static int adt7420_iio_setup(struct capi_i2c_controller_handle *i2c)
{
	static struct capi_i2c_device adt7420_i2c = {
		.address = ADT7420_CLICK_I2C_ADDR,
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	static struct iio_capi_device adt7420_iio;
	struct adt7420_init_param adt7420_param = {
		.interface_init.i2c_dev = &adt7420_i2c,
		/* 16-bit: 0.0078 C/LSB. */
		.resolution_setting = 1,
		.active_device = ID_ADT7420,
	};
	struct adt7420_dev *adt7420 = NULL;
	int ret;

	adt7420_i2c.controller = i2c;

	ret = adt7420_init(&adt7420, adt7420_param);
	if (ret)
		return ret;

	return iio_adt7420_register(&adt7420_iio, adt7420);
}

/**
 * @brief Bring up the ADXL345 on the Click socket, measuring at +/-2 g in
 *        full resolution, and add it to the IIO context.
 *
 * The Accel Click's COMM SEL jumpers put it on I2C or SPI: I2C is tried at
 * both addresses first, then SPI. It only reads the ID before it knows
 * the part, so an ADXL367 there is left as it is.
 * @param i2c   - The Click socket I2C controller, NULL for none.
 * @param spi   - The Click socket SPI controller, NULL for none.
 * @param taken - Set to where it was found.
 * @return 0 in case of success, negative error code otherwise.
 */
static int adxl345_iio_setup(struct capi_i2c_controller_handle *i2c,
			     struct capi_spi_controller_handle *spi,
			     struct accel_click_taken *taken)
{
	static struct capi_i2c_device adxl345_i2c = {
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	/* SS0, the socket CS; the ADXL345 takes SPI mode 3. */
	static struct capi_spi_device adxl345_spi = {
		.max_speed_hz = 1000000,
		.mode = CAPI_SPI_MODE_3,
		.native_cs = 1,
	};
	static struct iio_capi_device adxl345_iio;
	struct adxl345_init_param adxl345_param = {
		.dev_type = ID_ADXL345,
	};
	struct adxl345_dev *adxl345 = NULL;
	int ret = -ENODEV;
	size_t i;

	if (i2c) {
		adxl345_i2c.controller = i2c;
		adxl345_param.communication_type = ADXL345_I2C_COMM;
		adxl345_param.i2c_dev = &adxl345_i2c;
		for (i = 0; i < sizeof(accel_click_i2c_addr) && ret; i++) {
			adxl345_i2c.address = accel_click_i2c_addr[i];
			/* It gives the device also when the ID is not the
			 * part's. */
			ret = adxl345_init(&adxl345, adxl345_param);
			if (ret && adxl345)
				adxl345_remove(adxl345);
			if (!ret)
				taken->i2c_addr = adxl345_i2c.address;
		}
	}

	if (ret && spi) {
		adxl345_spi.controller = spi;
		adxl345_param.communication_type = ADXL345_SPI_COMM;
		adxl345_param.i2c_dev = NULL;
		adxl345_param.spi_dev = &adxl345_spi;
		adxl345 = NULL;
		ret = adxl345_init(&adxl345, adxl345_param);
		if (ret && adxl345)
			adxl345_remove(adxl345);
		if (!ret)
			taken->spi = true;
	}
	if (ret)
		return ret;

	/* The part has no soft reset and keeps its registers over a board
	 * reset: the offsets may be left from before, or from an ADXL367
	 * probe at its address. */
	adxl345_set_offset(adxl345, 0, 0, 0);
	adxl345_set_range_resolution(adxl345, ADXL345_RANGE_PM_2G,
				     ADXL345_FULL_RES);
	adxl345_set_power_mode(adxl345, 1);

	ret = iio_adxl345_register(&adxl345_iio, adxl345);
	if (ret)
		adxl345_remove(adxl345);

	return ret;
}

/**
 * @brief Bring up the ADXL367 on the Click socket, measuring at +/-2 g,
 *        and add it to the IIO context.
 *
 * The Accel 22 Click's COMM SEL jumpers put it on I2C or SPI: I2C is tried
 * at both addresses first, then SPI, but not where the ADXL345 was found:
 * the ADXL367 init writes its soft reset register before it reads the ID,
 * and on the ADXL345 that is OFSY.
 * @param i2c   - The Click socket I2C controller, NULL for none.
 * @param spi   - The Click socket SPI controller, NULL for none.
 * @param taken - Where the ADXL345 was found; set to where it was.
 * @return 0 in case of success, negative error code otherwise.
 */
static int adxl367_iio_setup(struct capi_i2c_controller_handle *i2c,
			     struct capi_spi_controller_handle *spi,
			     struct accel_click_taken *taken)
{
	static struct capi_i2c_device adxl367_i2c = {
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	/* SS0, the socket CS. */
	static struct capi_spi_device adxl367_spi = {
		.max_speed_hz = 1000000,
		.mode = CAPI_SPI_MODE_0,
		.native_cs = 1,
	};
	static struct iio_capi_device adxl367_iio;
	struct adxl367_init_param adxl367_param = {
		.id = ADXL367_ID,
	};
	struct adxl367_dev *adxl367 = NULL;
	int ret = -ENODEV;
	size_t i;

	if (i2c) {
		adxl367_i2c.controller = i2c;
		adxl367_param.comm_type = ADXL367_I2C_COMM;
		adxl367_param.i2c_dev = &adxl367_i2c;
		for (i = 0; i < sizeof(accel_click_i2c_addr) && ret; i++) {
			if (accel_click_i2c_addr[i] == taken->i2c_addr)
				continue;
			adxl367_i2c.address = accel_click_i2c_addr[i];
			ret = adxl367_init(&adxl367, adxl367_param);
		}
	}

	if (ret && spi && !taken->spi) {
		adxl367_spi.controller = spi;
		adxl367_param.comm_type = ADXL367_SPI_COMM;
		adxl367_param.i2c_dev = NULL;
		adxl367_param.spi_dev = &adxl367_spi;
		ret = adxl367_init(&adxl367, adxl367_param);
		if (!ret)
			taken->spi = true;
	}
	if (ret)
		return ret;

	ret = adxl367_set_range(adxl367, ADXL367_2G_RANGE);
	if (!ret)
		ret = adxl367_set_power_mode(adxl367, ADXL367_OP_MEASURE);
	if (!ret)
		ret = iio_adxl367_register(&adxl367_iio, adxl367);
	if (ret)
		adxl367_remove(adxl367);

	return ret;
}

/**
 * @brief Bring up the AD7091R on the Click socket SPI, with CONVST on the
 *        socket RST pin, and add it to the IIO context.
 *
 * The AD7091R has no ID to read back, so it is added whenever the SPI is
 * not taken by an accelerometer, also when no ISO ADC 2 Click is there.
 * @param spi - The Click socket SPI controller.
 * @return 0 in case of success, negative error code otherwise.
 */
static int ad7091r_iio_setup(struct capi_spi_controller_handle *spi)
{
	/* SS0, the socket CS. */
	static struct capi_spi_device ad7091r_spi = {
		.max_speed_hz = 1000000,
		.mode = CAPI_SPI_MODE_0,
		.native_cs = 1,
	};
	static struct iio_ad7091r ad7091r_iio;
	struct capi_gpio_port_handle *ctl_port = NULL;
	struct ad7091r_init_param ad7091r_param = {
		.spi_dev = &ad7091r_spi,
		/* ISO ADC 2 Click: CONVST is on the socket RST pin. */
		.convst = &bsp_click_rst,
	};
	struct ad7091r_dev *ad7091r = NULL;
	int ret;

	ret = capi_gpio_port_init(&ctl_port, &bsp_click_ctl_port_cfg);
	if (ret)
		return ret;
	bsp_click_rst.port_handle = ctl_port;

	ad7091r_spi.controller = spi;
	ret = ad7091r_init(&ad7091r, ad7091r_param);
	if (ret)
		return ret;

	ret = iio_ad7091r_register(&ad7091r_iio, ad7091r,
				   AD7091R_CLICK_VREF_MV);
	if (ret)
		ad7091r_remove(ad7091r);

	return ret;
}

/**
 * @brief Initialize the console UART and the Click parts, then serve the
 *        IIO context.
 * @return A negative error code. It does not return while the context is
 *         served.
 */
int main(void)
{
	struct capi_i2c_controller_handle *i2c = NULL;
	struct capi_spi_controller_handle *spi = NULL;
	struct capi_uart_handle *uart = NULL;
	struct accel_click_taken taken = {0};
	int ret;

	capi_platform_init();

	/* The console UART carries the IIO protocol: no capi_stdio_init(). */
	/* bsp_uart_cfg comes from the board bsp (parameters.h). */
	ret = capi_uart_init(&uart, &bsp_uart_cfg);
	if (ret)
		return ret;

	/* A part that does not come up is left out of the context, which a
	 * client can still open. */
	if (capi_i2c_init(&i2c, &bsp_i2c_cfg))
		i2c = NULL;
	if (capi_spi_init(&spi, &bsp_spi_cfg))
		spi = NULL;

	if (i2c)
		adt7420_iio_setup(i2c);
	adxl345_iio_setup(i2c, spi, &taken);
	adxl367_iio_setup(i2c, spi, &taken);
	if (spi && !taken.spi)
		ad7091r_iio_setup(spi);

	return iiod_uart_run(uart, NULL);
}
