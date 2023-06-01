cmake_minimum_required(VERSION 3.16)

message(STATUS "$ENV{PICO_SDK_PATH}")

set(PPATH "$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

include(${PPATH})

project(test)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()


add_executable(test
        main.c
        )
                          
add_subdirectory("dht22")

# pull in common dependencies
target_link_libraries(test dht22 pico_stdlib hardware_pio)

# enable usb output, disable uart output
pico_enable_stdio_usb(test 1)
pico_enable_stdio_uart(test 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(test)

