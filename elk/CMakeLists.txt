cmake_minimum_required(VERSION 3.24)

set(CMAKE_BUILD_TYPE Debug)
# set(PICO_DEOPTIMIZED_DEBUG 1)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(elk)

pico_sdk_init()

add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_pio)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/clk6502.pio)
pico_add_extra_outputs(${PROJECT_NAME})
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
