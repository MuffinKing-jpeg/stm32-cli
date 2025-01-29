cmake_minimum_required(VERSION 3.22)

project(cli)
add_library(cli INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

message("CLI lib by:
Andrii Dobrus
Git: https://github.com/MuffinKing-jpeg")

file(GLOB CLI_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}//Src/*.c)

target_include_directories(cli INTERFACE
${CMAKE_CURRENT_SOURCE_DIR}//Inc
)

target_sources(cli PUBLIC
    ${CLI_SRC_FILES}
    # Add user sources here
)

target_link_directories(cli INTERFACE
)

target_link_libraries(cli INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()