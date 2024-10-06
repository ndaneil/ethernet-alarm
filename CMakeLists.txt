# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C++ standards
project(alarm C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(ARCHIVE_OUTPUT_DIRECTORY ./build)

# WIZnet libraries
set(BOARD_NAME W55RP20_EVB_PICO)
set(WIZNET_CHIP W5500)
add_definitions(-D_WIZCHIP_=W5500)
add_definitions(-DDEVICE_BOARD_NAME=W55RP20_EVB_PICO)
message(STATUS "WIZNET_CHIP = ${WIZNET_CHIP}")
include(${CMAKE_SOURCE_DIR}/../c-sdk/wiznet_pico_c_sdk_version.cmake)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(alarm
    main.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(alarm)
pico_add_uf2_output(alarm)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(alarm PRIVATE
    pico_stdlib
    hardware_spi
    hardware_dma
    ETHERNET_FILES
    IOLIBRARY_FILES
    DHCP_FILES
    DNS_FILES
    TIMER_FILES
    HTTPSERVER_FILES
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(alarm 1)
pico_enable_stdio_uart(alarm 0)


# assume built-in pthreads on MacOS, remove for other platforms
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Set the project root directory if it's not already defined, as may happen if
# the tests folder is included directly by a parent project, without including
# the top level CMakeLists.txt.
if(NOT DEFINED WIZNET_DIR)
    set(WIZNET_DIR ${CMAKE_SOURCE_DIR}/../c-sdk/libraries/ioLibrary_Driver)
    message(STATUS "WIZNET_DIR = ${WIZNET_DIR}")
endif()

if(NOT DEFINED MBEDTLS_DIR)
    set(MBEDTLS_DIR ${CMAKE_SOURCE_DIR}/../c-sdk/libraries/mbedtls)
    message(STATUS "MBEDTLS_DIR = ${MBEDTLS_DIR}")
endif()

if(NOT DEFINED PORT_DIR)
    set(PORT_DIR ${CMAKE_SOURCE_DIR}/../c-sdk/port)
    message(STATUS "PORT_DIR = ${PORT_DIR}")
endif()

# Turn off mbedtls test mode 
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build mbedtls programs")
set(ENABLE_TESTING OFF CACHE BOOL "Build mbedtls testing")
add_definitions(-DMBEDTLS_CONFIG_FILE="${PORT_DIR}/mbedtls/inc/ssl_config.h")
add_definitions(-DSET_TRUSTED_CERT_IN_SAMPLES)

# Add libraries in subdirectories
add_subdirectory(${CMAKE_SOURCE_DIR}/../c-sdk/libraries ./build/lib)
add_subdirectory(${MBEDTLS_DIR} ./build/lib-mdbedtls)
add_subdirectory(${PORT_DIR} ./build/lib-port)

# Set compile options
add_compile_options(
        -Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )
