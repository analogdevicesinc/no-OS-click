# util

The no-OS utilities that the drivers in `drivers/` use. These are the
platform-independent helpers only: bit and field macros, math, unaligned
access, error codes and log macros. The platform services are not here:
peripherals (`no_os_spi`, `no_os_i2c`, `no_os_gpio`, `no_os_delay`) and
allocation (`no_os_alloc`). The drivers use the CAPI equivalents instead,
e.g. `capi_malloc()`/`capi_free()` from `capi_alloc.h`.

The plan is to move this directory to its own capi-util repository.

## Contents

| File | What the drivers use from it |
|---|---|
| `include/no_os_util.h`, `src/no_os_util.c` | `NO_OS_BIT`, `NO_OS_GENMASK`, `no_os_field_prep/get`, `no_os_hweight16`, `no_os_div_s64_rem`, `no_os_get_unaligned_be16` |
| `include/no_os_error.h` | `<errno.h>` and the `NO_OS_IS_ERR*` helpers |
| `include/no_os_print_log.h` | `pr_info`, `pr_err`, ... (`printf`, gated by `NO_OS_LOG_LEVEL`) |

## Origin

Copied unchanged from
[no-OS](https://github.com/analogdevicesinc/no-OS) at commit `b87ffaf4f`
(`include/` and `util/`). The files depend only on the C library.

`no_os_print_log.h` calls `no_os_get_time()` from `no_os_delay.h` when
`PRINT_TIME` is defined. Leave `PRINT_TIME` undefined.
