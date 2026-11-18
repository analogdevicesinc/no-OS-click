# no-os-click

A [CAPI](https://github.com/analogdevicesinc/capi) application for Click
boards, built on the drivers in this repository.

It probes an ADXL345 (I2C address 0x53, else 0x1D), an ADXL367 (0x1D), an
ADT7420 (0x48) and an AD5593R (0x10) on the Click socket I2C bus, and the
ADXL367 also on the Click socket SPI bus if it is not on I2C. For each one
found, it prints a reading on the console UART once a second: acceleration from
the ADXL345 in g and from the ADXL367 in m/s^2, and temperature from the ADT7420
and from the AD5593R's temperature indicator in degrees C. A device that is not
found is skipped.

Unless the ADXL367 is on the SPI bus, it also samples an AD7091R (ISO ADC 2
Click) there, with the socket CS as the controller's native chip-select and
CONVST on the socket RST pin, and prints the input voltage against the 2.5 V
internal reference. The AD7091R has no ID to read back, so the application asks
on the console whether the Click is on the socket; press `y` if it is.

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
| Alif | `dk-b1-rtss-he` | `build-alif/dk-b1` |
| Alif | `dk-e1c-rtss-he` | `build-alif/dk-e1c` |

## Adding a driver

Add the driver's path under `drivers/` to `NO_OS_CLICK_DRIVERS` in
`CMakeLists.txt`:

```cmake
set(NO_OS_CLICK_DRIVERS
    accel/adxl345
    accel/adxl367
    adc/ad7091r
    adc-dac/ad5592r
    temperature/adt7420
)
```

The build compiles every `.c` file in that directory and adds the directory
to the include path. The no-OS utilities from `util/` are always built in.

## License

BSD-3-Clause; see [LICENSE](../../LICENSE).
