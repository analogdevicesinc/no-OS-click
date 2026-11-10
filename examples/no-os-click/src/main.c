/*
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***************************************************************************//**
 *   @file   main.c
 *   @brief  Click board application for CAPI.
 *
 *   Probe an ADXL345, an ADXL367, an ADT7420 and an AD5593R on the Click
 *   socket I2C bus and print the reading of each one found on the console
 *   UART once a second.
*******************************************************************************/
#include "capi_platform.h"
#include "capi_time.h"
#include "capi_uart.h"
#include "capi_i2c.h"
#include "parameters.h"
#include "adxl345.h"
#include "adxl367.h"
#include "adt7420.h"
#include "ad5593r.h"
#include <stdio.h>

/* SDO/ALT ADDRESS low: 0x53. With it high, use ADXL345_ADDRESS (0x1D). */
#define ADXL345_CLICK_I2C_ADDR	0x53
/* ASEL high: 0x1D. With it low, 0x53. */
#define ADXL367_CLICK_I2C_ADDR	0x1D
/* A1 = A0 = 0. */
#define ADT7420_CLICK_I2C_ADDR	ADT7420_ADDRESS(0, 0)
/* A0 low: 0x10. With it high, 0x11. */
#define AD5593R_CLICK_I2C_ADDR	0x10

/**
 * @brief Convert an AD5593R temperature code to degrees Celsius.
 *
 * For the 0 to VREF ADC range with the 2.5 V internal reference
 * (AD5593R datasheet, Temperature Indicator).
 *
 * @param code - The 12-bit code read from AD5592R_TEMPERATURE_CHANNEL.
 * @return The temperature in degrees Celsius.
 */
static float ad5593r_code_to_celsius(uint16_t code)
{
	return 25.0f + ((float)code - 819.0f) / 2.654f;
}

/**
 * @brief Convert an ADXL367 fractional value (10^-9 units) to a float.
 * @param val - The value.
 * @return The value as a float.
 */
static float adxl367_val_to_float(const struct adxl367_fractional_val *val)
{
	return (float)val->integer + (float)val->fractional / 1000000000.0f;
}

/**
 * @brief Initialize the console UART and the sensors, then print their
 *        readings.
 * @return This function does not return.
 */
int main(void)
{
	struct capi_uart_handle *uart = NULL;
	struct capi_i2c_controller_handle *i2c = NULL;
	struct capi_i2c_device adxl345_i2c = {
		.address = ADXL345_CLICK_I2C_ADDR,
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	struct capi_i2c_device adxl367_i2c = {
		.address = ADXL367_CLICK_I2C_ADDR,
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	struct capi_i2c_device adt7420_i2c = {
		.address = ADT7420_CLICK_I2C_ADDR,
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	struct capi_i2c_device ad5593r_i2c = {
		.address = AD5593R_CLICK_I2C_ADDR,
		.speed = CAPI_I2C_SPEED_STANDARD,
	};
	struct adxl345_init_param adxl345_param = {
		.i2c_dev = &adxl345_i2c,
		.dev_type = ID_ADXL345,
		.communication_type = ADXL345_I2C_COMM,
	};
	struct adxl367_init_param adxl367_param = {
		.i2c_dev = &adxl367_i2c,
		.id = ADXL367_ID,
		.comm_type = ADXL367_I2C_COMM,
	};
	struct adt7420_init_param adt7420_param = {
		.interface_init.i2c_dev = &adt7420_i2c,
		/* 16-bit: 0.0078 C/LSB. */
		.resolution_setting = 1,
		.active_device = ID_ADT7420,
	};
	/* All I/O channels unused: only the temperature indicator is read. */
	struct ad5592r_init_param ad5593r_param = {
		.int_ref = true,
		.i2c_dev = &ad5593r_i2c,
		.adc_range = ZERO_TO_VREF,
		.dac_range = ZERO_TO_VREF,
	};
	struct adxl345_dev *adxl345 = NULL;
	struct adxl367_dev *adxl367 = NULL;
	struct adt7420_dev *adt7420 = NULL;
	struct ad5592r_dev *ad5593r = NULL;
	uint16_t code;
	struct adxl367_fractional_val x367, y367, z367;
	float x, y, z;
	int ret;

	capi_platform_init();

	/* bsp_uart_cfg comes from the board bsp (parameters.h). */
	capi_uart_init(&uart, &bsp_uart_cfg);
	/* printf() reaches the console UART only after capi_stdio_init(). */
	capi_stdio_init(uart);

	printf("\r\n*** no-os-click: ADXL345 + ADXL367 + ADT7420 + AD5593R over I2C ***\r\n\r\n");

	ret = capi_i2c_init(&i2c, &bsp_i2c_cfg);
	if (ret) {
		printf("Click I2C init failed (%d)\r\n", ret);
		goto halt;
	}
	adxl345_i2c.controller = i2c;
	adxl367_i2c.controller = i2c;
	adt7420_i2c.controller = i2c;
	ad5593r_i2c.controller = i2c;

	ret = adxl345_init(&adxl345, adxl345_param);
	if (ret) {
		printf("ADXL345 not found at 0x%02x (%d)\r\n",
		       ADXL345_CLICK_I2C_ADDR, ret);
		adxl345 = NULL;
	} else {
		adxl345_set_range_resolution(adxl345, ADXL345_RANGE_PM_2G,
					     ADXL345_FULL_RES);
		adxl345_set_power_mode(adxl345, 1);
		printf("ADXL345 found at 0x%02x\r\n", ADXL345_CLICK_I2C_ADDR);
	}

	ret = adxl367_init(&adxl367, adxl367_param);
	if (ret) {
		printf("ADXL367 not found at 0x%02x (%d)\r\n",
		       ADXL367_CLICK_I2C_ADDR, ret);
		adxl367 = NULL;
	} else {
		ret = adxl367_set_range(adxl367, ADXL367_2G_RANGE);
		if (!ret)
			ret = adxl367_set_power_mode(adxl367, ADXL367_OP_MEASURE);
		if (ret) {
			printf("ADXL367 setup failed (%d)\r\n", ret);
			adxl367 = NULL;
		} else {
			printf("ADXL367 found at 0x%02x\r\n",
			       ADXL367_CLICK_I2C_ADDR);
		}
	}

	ret = adt7420_init(&adt7420, adt7420_param);
	if (ret) {
		printf("ADT7420 not found at 0x%02x (%d)\r\n",
		       ADT7420_CLICK_I2C_ADDR, ret);
		adt7420 = NULL;
	} else {
		printf("ADT7420 found at 0x%02x\r\n", ADT7420_CLICK_I2C_ADDR);
	}

	ret = ad5593r_init(&ad5593r, &ad5593r_param);
	if (ret) {
		printf("AD5593R not found at 0x%02x (%d)\r\n",
		       AD5593R_CLICK_I2C_ADDR, ret);
		ad5593r = NULL;
	} else {
		printf("AD5593R found at 0x%02x\r\n", AD5593R_CLICK_I2C_ADDR);
	}

	if (!adxl345 && !adxl367 && !adt7420 && !ad5593r)
		goto halt;

	printf("\r\n");

	while (1) {
		if (adxl345) {
			adxl345_get_g_xyz(adxl345, &x, &y, &z);
			printf("ADXL345: x = %6.3f g,     y = %6.3f g,     z = %6.3f g\r\n",
			       x, y, z);
		}
		if (adxl367) {
			ret = adxl367_get_g_xyz(adxl367, &x367, &y367, &z367);
			if (ret)
				printf("ADXL367: read failed (%d)\r\n", ret);
			else
				printf("ADXL367: x = %6.3f m/s^2, y = %6.3f m/s^2, z = %6.3f m/s^2\r\n",
				       adxl367_val_to_float(&x367),
				       adxl367_val_to_float(&y367),
				       adxl367_val_to_float(&z367));
		}
		if (adt7420)
			printf("ADT7420: t = %7.3f C\r\n",
			       adt7420_get_temperature(adt7420));
		if (ad5593r) {
			ret = ad5593r_read_adc(ad5593r,
					       AD5592R_TEMPERATURE_CHANNEL, &code);
			if (ret)
				printf("AD5593R: read failed (%d)\r\n", ret);
			else
				printf("AD5593R: t = %7.3f C (code %u)\r\n",
				       ad5593r_code_to_celsius(code), code);
		}
		capi_wait_ms(1000);
	}

halt:
	while (1)
		capi_wait_ms(1000);

	return 0;
}
