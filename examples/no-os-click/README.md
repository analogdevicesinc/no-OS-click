# no-os-click

A [CAPI](https://github.com/analogdevicesinc/capi) app for Click boards. It
uses the drivers in this repository's `drivers/` directory, ported from the
no-OS API to CAPI.

It probes an ADXL345 (I2C address 0x53), an ADXL367 (0x1D), an ADT7420 (0x48)
and an AD5593R (0x10) on the Click socket I2C bus. For each one found, it prints
a reading on the console UART once a second: acceleration from the ADXL345 in g
and from the ADXL367 in m/s^2, and temperature from the ADT7420 and from the
AD5593R's temperature indicator in degrees C. A device that is not found is
skipped.

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

The first configure clones capi-alif into the workspace.

## Platform setup

**Alif.** Install `arm-none-eabi-gcc` with newlib. To flash, set
`ALIF_SETOOLS_PATH`.

```bash
export ALIF_CMSIS_DFP_PATH=/path/to/alif_ensemble-cmsis-dfp
export CMSIS_6_PATH=/path/to/CMSIS_6
```

## Build and flash

Run in `~/workspace/no-OS-click/examples/no-os-click`:

```bash
cmake --preset <preset>
cmake --build <build dir> --target flash
```

| Platform | Preset | Build dir |
|---|---|---|
| Alif | `dk-e8-rtss-hp`, `dk-e8-rtss-he` | `build-alif/rtss_hp`, `build-alif/rtss_he` |

## Adding a driver

Once a driver is ported to CAPI, add its path under `drivers/` to
`NO_OS_CLICK_DRIVERS` in `CMakeLists.txt`:

```cmake
set(NO_OS_CLICK_DRIVERS
    accel/adxl345
    accel/adxl367
    adc-dac/ad5592r
    temperature/adt7420
)
```

The build compiles every `.c` file in that directory and adds the directory
to the include path. The no-OS utilities the drivers use (`no_os_util.h`,
`no_os_error.h`, ...) come from `util/` at the repository root and are always
built in.

## License

BSD-3-Clause.
