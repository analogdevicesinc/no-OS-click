# Copyright (c) 2026 Analog Devices, Inc.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.27)

# CAPI_DIR is the capi checkout.
# Set it as an environment variable or with -DCAPI_DIR=<path>.
if(NOT DEFINED CAPI_DIR)
    if(NOT DEFINED ENV{CAPI_DIR})
        message(FATAL_ERROR
            "CAPI_DIR not set. Export CAPI_DIR=/path/to/capi or pass -DCAPI_DIR=...")
    endif()
    set(CAPI_DIR "$ENV{CAPI_DIR}")
endif()

include("${CAPI_DIR}/sdk/cmake/capi_create_application.cmake")

# capi_resolve_platform() sets CMAKE_TOOLCHAIN_FILE for the platform that
# PLATFORM selects. Call it before project(), because CMake reads the
# toolchain file there. It also resolves the packages in module.yml, which
# gives LIBIIO_DIR, and the two checkouts the Alif toolchain file asks for.
capi_resolve_platform()
if(NOT DEFINED ENV{ALIF_CMSIS_DFP_PATH})
    set(ENV{ALIF_CMSIS_DFP_PATH} "${ALIF_ENSEMBLE_CMSIS_DFP_DIR}")
endif()
if(NOT DEFINED ENV{CMSIS_6_PATH})
    set(ENV{CMSIS_6_PATH} "${CMSIS_6_DIR}")
endif()
project(no_os_click_iio LANGUAGES C ASM)

# The drivers and their IIO support, in iio/, from the drivers/ directory at
# the root of this repository. List each one by its path there.
get_filename_component(NO_OS_CLICK_DRIVERS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../drivers" ABSOLUTE)
set(NO_OS_CLICK_DRIVERS
    accel/adxl345
    accel/adxl367
    adc/ad7091r
    temperature/adt7420
)

set(NO_OS_CLICK_DRIVER_SRCS "")
set(NO_OS_CLICK_DRIVER_INCS "")
foreach(_drv IN LISTS NO_OS_CLICK_DRIVERS)
    set(_drv_dir "${NO_OS_CLICK_DRIVERS_DIR}/${_drv}")
    if(NOT IS_DIRECTORY "${_drv_dir}/iio")
        message(FATAL_ERROR
            "no-os-click: driver '${_drv}' has no iio/ at ${_drv_dir}")
    endif()
    file(GLOB _drv_srcs CONFIGURE_DEPENDS
        "${_drv_dir}/*.c" "${_drv_dir}/iio/*.c")
    list(APPEND NO_OS_CLICK_DRIVER_SRCS ${_drv_srcs})
    list(APPEND NO_OS_CLICK_DRIVER_INCS "${_drv_dir}" "${_drv_dir}/iio")
endforeach()

# The no-OS utilities the driver uses, from the util/ directory at the root
# of this repository.
get_filename_component(NO_OS_CLICK_UTIL_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../util" ABSOLUTE)
file(GLOB NO_OS_CLICK_UTIL_SRCS CONFIGURE_DEPENDS "${NO_OS_CLICK_UTIL_DIR}/src/*.c")

capi_create_application(no_os_click_iio
    SOURCES src/main.c ${NO_OS_CLICK_DRIVER_SRCS} ${NO_OS_CLICK_UTIL_SRCS})
target_include_directories(no_os_click_iio PRIVATE
    "${NO_OS_CLICK_UTIL_DIR}/include" ${NO_OS_CLICK_DRIVER_INCS})

# libiio's CAPI port, the package libiio in module.yml. It serves an IIO
# context to libiio clients.
add_subdirectory("${LIBIIO_DIR}" "${CMAKE_BINARY_DIR}/libiio")
target_link_libraries(no_os_click_iio PRIVATE libiio::capi)
