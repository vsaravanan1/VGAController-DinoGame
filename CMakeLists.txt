# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(VGAController C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(VGAController VGAController.c )

pico_set_program_name(VGAController "VGAController")
pico_set_program_version(VGAController "0.1")

# Generate PIO header
pico_generate_pio_header(VGAController ${CMAKE_CURRENT_LIST_DIR}/hsync.pio)
pico_generate_pio_header(VGAController ${CMAKE_CURRENT_LIST_DIR}/vsync.pio)
pico_generate_pio_header(VGAController ${CMAKE_CURRENT_LIST_DIR}/rgb.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(VGAController 1)
pico_enable_stdio_usb(VGAController 1)


# Add the standard include files to the build
target_include_directories(VGAController PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(VGAController PRIVATE
        pico_stdlib
        hardware_dma
        hardware_pio
        pico_divider 
        pico_multicore 
        pico_bootsel_via_double_reset 
        hardware_adc 
        hardware_irq 
        hardware_clocks 
        hardware_pll
)

pico_add_extra_outputs(VGAController)

