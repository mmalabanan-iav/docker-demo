# Source files
set(SOURCES
    main.c
)

# Create executable
add_executable(${PROJECT_NAME}.elf ${SOURCES})

# Set output directory to build root instead of build/src
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Link with libopencm3
target_link_libraries(${PROJECT_NAME}.elf 
    opencm3_stm32l4
)

# Linker script and library path
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/stm32l432.ld)
target_link_options(${PROJECT_NAME}.elf PRIVATE 
    -T${LINKER_SCRIPT}
    -L${CMAKE_SOURCE_DIR}/libopencm3/lib
)

# Make sure linker script exists
set_target_properties(${PROJECT_NAME}.elf PROPERTIES 
    LINK_DEPENDS ${LINKER_SCRIPT}
)

# Generate binary file
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Creating binary file ${PROJECT_NAME}.bin"
)

# Show size information
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
    COMMENT "Size information:"
)