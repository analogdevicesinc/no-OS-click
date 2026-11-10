# no-OS-click

Drivers for the Analog Devices parts on MikroElektronika Click boards, ported
from [no-OS](https://github.com/analogdevicesinc/no-OS) to
[CAPI](https://github.com/analogdevicesinc/capi), and a CAPI application that
uses them.

## Drivers

| Driver | Parts | Bus | Click board |
|---|---|---|---|
| `drivers/accel/adxl345` | ADXL345, ADXL346 | I2C, SPI | Accel Click |
| `drivers/accel/adxl367` | ADXL367, ADXL366 | I2C, SPI | Accel 22 Click |
| `drivers/temperature/adt7420` | ADT7420, ADT7410, ADT7422 (I2C); ADT7310, ADT7311, ADT7312, ADT7320 (SPI) | I2C, SPI | Surface Temp 2 Click |
| `drivers/adc-dac/ad5592r` | AD5593R (I2C), AD5592R (SPI) | I2C, SPI | ADAC Click |

The drivers use CAPI for the bus, delays and memory allocation, and keep the
no-OS API otherwise. They are tested over I2C on an Alif DK-E8 (RTSS-HP). The
SPI paths build but are not tested yet.

The application owns the bus: it initializes the SPI or I2C controller and
passes the driver a `capi_spi_device` or `capi_i2c_device`. The driver does
not initialize or remove the bus, and leaves the SPI chip-select to the
platform. For example:

```c
struct capi_i2c_controller_handle *i2c = NULL;
struct capi_i2c_device i2c_dev = {
	.address = 0x53,
	.speed = CAPI_I2C_SPEED_STANDARD,
};
struct adxl345_init_param param = {
	.i2c_dev = &i2c_dev,
	.dev_type = ID_ADXL345,
	.communication_type = ADXL345_I2C_COMM,
};
struct adxl345_dev *adxl345;

capi_i2c_init(&i2c, &bsp_i2c_cfg);
i2c_dev.controller = i2c;
adxl345_init(&adxl345, param);
```

## Layout

| Directory | Contents |
|---|---|
| `drivers/` | The drivers, one directory each |
| `util/` | The no-OS utilities the drivers use (`no_os_util`, `no_os_error`, `no_os_print_log`); see [util/README.md](util/README.md) |
| `examples/no-os-click/` | A CAPI application that reads the parts above; see [examples/no-os-click/README.md](examples/no-os-click/README.md) |

## CAPI setup

You need CMake 3.27+, Ninja, Python 3 and network access. Clone capi and this
repository into one workspace:

```bash
mkdir ~/workspace && cd ~/workspace
git clone https://github.com/analogdevicesinc/capi
git clone https://github.com/analogdevicesinc/no-OS-click
```

```bash
export CAPI_WORKSPACE=~/workspace
export CAPI_DIR=~/workspace/capi
```

The first configure of an application clones its platform (capi-alif) into the
workspace.

## Platform setup

**Alif.** Install `arm-none-eabi-gcc` with newlib. To flash, set
`ALIF_SETOOLS_PATH`.

```bash
export ALIF_CMSIS_DFP_PATH=/path/to/alif_ensemble-cmsis-dfp
export CMSIS_6_PATH=/path/to/CMSIS_6
```

## License

BSD-3-Clause; see [LICENSE](LICENSE).
