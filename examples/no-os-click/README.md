# no-os-click

A [CAPI](https://github.com/analogdevicesinc/capi) application for Click
boards, built on the drivers in this repository.

It probes an ADXL345 (I2C address 0x53), an ADXL367 (0x1D), an ADT7420 (0x48)
and an AD5593R (0x10) on the Click socket I2C bus. For each one found, it prints
a reading on the console UART once a second: acceleration from the ADXL345 in g
and from the ADXL367 in m/s^2, and temperature from the ADT7420 and from the
AD5593R's temperature indicator in degrees C. A device that is not found is
skipped.

## Build and flash

Set up CAPI and the platform first; see the
[repository README](../../README.md#capi-setup). Then run in
`~/workspace/no-OS-click/examples/no-os-click`:

```bash
cmake --preset <preset>
cmake --build <build dir> --target flash
```

| Platform | Preset | Build dir |
|---|---|---|
| Alif | `dk-e8-rtss-hp`, `dk-e8-rtss-he` | `build-alif/rtss_hp`, `build-alif/rtss_he` |

## Adding a driver

Add the driver's path under `drivers/` to `NO_OS_CLICK_DRIVERS` in
`CMakeLists.txt`:

```cmake
set(NO_OS_CLICK_DRIVERS
    accel/adxl345
    accel/adxl367
    adc-dac/ad5592r
    temperature/adt7420
)
```

The build compiles every `.c` file in that directory and adds the directory
to the include path. The no-OS utilities from `util/` are always built in.

## License

BSD-3-Clause; see [LICENSE](../../LICENSE).
