

set(PPATH "$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")

include(${PPATH})

# Initialize the SDK
pico_sdk_init()

add_library(dht22 STATIC dht22.c)

pico_generate_pio_header(dht22 dht22.pio)

target_link_libraries(dht22 pico_stdlib hardware_pio)

target_include_directories(dht22
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	)
