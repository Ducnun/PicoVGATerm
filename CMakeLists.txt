cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_project C CXX ASM)

pico_sdk_init()

add_executable(pico_project
    src/main.c
)

target_link_libraries(pico_project
    pico_stdlib
)

pico_enable_stdio_usb(pico_project 1)
pico_enable_stdio_uart(pico_project 0)

pico_add_extra_outputs(pico_project)