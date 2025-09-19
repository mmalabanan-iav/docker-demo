cmake_minimum_required(VERSION 3.20)

# Set the toolchain file before project() call
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-toolchain.cmake)

# Project definition
project(docker-demo VERSION 1.0.0 LANGUAGES C ASM)

# STM32 specific definitions
add_compile_definitions(STM32L4)

# Compiler flags
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Os -g3 -Wall -Wextra)

# Include directories
include_directories(src)
include_directories(libopencm3/include)

# Build libopencm3 using its existing Makefile system
set(LIBOPENCM3_LIB ${CMAKE_SOURCE_DIR}/libopencm3/lib/libopencm3_stm32l4.a)

# Custom command to build libopencm3
add_custom_command(
    OUTPUT ${LIBOPENCM3_LIB}
    COMMAND make -C ${CMAKE_SOURCE_DIR}/libopencm3 "TARGETS=stm32/l4" "DEVICE=stm32l432kcu6"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/libopencm3
    COMMENT "Building libopencm3 for STM32L4"
    VERBATIM
)

# Create a custom target that depends on the library
add_custom_target(libopencm3_build DEPENDS ${LIBOPENCM3_LIB})

# Create an imported library to link against
add_library(opencm3_stm32l4 STATIC IMPORTED GLOBAL)
set_target_properties(opencm3_stm32l4 PROPERTIES
    IMPORTED_LOCATION ${LIBOPENCM3_LIB}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/libopencm3/include
)

# Make sure the library is built before it's used
add_dependencies(opencm3_stm32l4 libopencm3_build)

# Main executable
add_subdirectory(src)
