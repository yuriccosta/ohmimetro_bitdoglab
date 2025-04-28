# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(ohmimetro C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(ohmimetro ohmimetro.c lib/ssd1306.c)

pico_set_program_name(ohmimetro "ohmimetro")
pico_set_program_version(ohmimetro "0.1")


# Generate PIO header
pico_generate_pio_header(ohmimetro ${CMAKE_CURRENT_LIST_DIR}/lib/animacao_matriz.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(ohmimetro 1)
pico_enable_stdio_usb(ohmimetro 1)

# Add the standard library to the build
target_link_libraries(ohmimetro
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(ohmimetro PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(ohmimetro 
        hardware_adc
        hardware_i2c
        hardware_clocks
        hardware_pio
        )

pico_add_extra_outputs(ohmimetro)

