# Minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(Engine3D
    VERSION 1.0.0 
    DESCRIPTION "A console-based game engine for Linux"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)

# Find ncurses library
pkg_check_modules(NCURSES REQUIRED ncurses)

# Find threading library
find_package(Threads REQUIRED)


# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${NCURSES_INCLUDE_DIRS})


add_library(Engine3D STATIC
    src/EngineBackend.cpp
    include/EngineBackend.hpp  # Headers are optional here
)

target_include_directories(Engine3D PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${NCURSES_INCLUDE_DIRS}
)

# Remove the .hpp from target_link_libraries
target_link_libraries(Engine3D PUBLIC
    ${NCURSES_LIBRARIES}
    Threads::Threads
)

target_compile_options(Engine3D PUBLIC
    ${NCURSES_CFLAGS_OTHER}
)

# Set compiler flags for the library
target_compile_options(Engine3D PUBLIC
    ${NCURSES_CFLAGS_OTHER}
)

# Example game executable
add_executable(ExampleGame
    src/Engine3D.cpp
)

# Link the engine to the example
target_link_libraries(ExampleGame
    Engine3D
)

# Install headers
install(DIRECTORY include/
    DESTINATION include/EngineBackend
    FILES_MATCHING PATTERN "*.h"
)

# Create uninstall target
# if(NOT TARGET uninstall)
#     configure_file(
#         "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#         "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#         IMMEDIATE @ONLY
#     )
#     add_custom_target(uninstall
#         COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
#     )
# endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
if(ALSA_FOUND)
    message(STATUS "ALSA support: Enabled")
else()
    message(STATUS "ALSA support: Disabled")
endif()
