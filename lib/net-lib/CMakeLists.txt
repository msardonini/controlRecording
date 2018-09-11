# CMake file for common library

#-------------------------------------------------------------------------------
# Options
##

# Set the minimum cmake version
cmake_minimum_required(VERSION 2.6)

option(GLOBAL_FOR_TEGRA "GLOBAL_FOR_TEGRA" OFF)             # compiles for the tegra
option(GLOBAL_FOR_RPI "GLOBAL_FOR_RPI" OFF)                 # compiles for the raspberry pi
option(GLOBAL_WITH_TESTING "GLOBAL_WITH_TESTING" OFF)       # compiles with testing
option(GLOBAL_WITH_DEBUG "GLOBAL_WITH_DEBUG" OFF)           # compiles with debugging
option(GLOBAL_WITH_EXAMPLES "GLOBAL_WITH_EXAMPLES" OFF)     # compiles with examples
    
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(NOT HAS_PARENT)

    if(GLOBAL_FOR_TEGRA)
        message(STATUS "Compiling for TEGRA")
        set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
        set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    endif()

    if(GLOBAL_FOR_RPI)
        message(STATUS "Compiling for Raspberry Pi")
        set(CMAKE_CXX_COMPILER arm-unknown-linux-gnueabihf-g++)
        set(CMAKE_C_COMPILER arm-unknown-linux-gnueabihf-gcc)
    endif()

    if(GLOBAL_WITH_TESTING)
        message(STATUS "Compiling with testing")
    endif()

    if(GLOBAL_WITH_EXAMPLES)
        message(STATUS "Compiling with examples")
    endif()

endif()


#-------------------------------------------------------------------------------
# Project
##

# project(NETLIB)

# Define the root MIDAS directory passed in through the build system
if (DEFINED MIDAS_ROOT)
    set(NETLIB_MIDAS_ROOT ${MIDAS_ROOT})
else()
    set(NETLIB_MIDAS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/external)
endif()

# Define this directory 
set(NETLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(NETLIB_BIN_DIR "${NETLIB_ROOT}/build")
set(NETLIB_INSTALL_DIR "${NETLIB_ROOT}/install")

# Add the cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH "${NETLIB_ROOT}/cmake")

# Contains the version string of the project
set(NETLIB_VERSION_MAJOR 0)

# Contains the subversion string of the project
set(NETLIB_VERSION_MINOR 4)

# Contains the patch version of the project
set(NETLIB_VERSION_PATCH 1)

# String of the version
set(NETLIB_VERSION_STRING ${NETLIB_VERSION_MAJOR}.${NETLIB_VERSION_MINOR}.${NETLIB_VERSION_PATCH})


#-------------------------------------------------------------------------------
# Compile options
##

if (NOT HAS_PARENT)

    # Add debugging symbols if needed
    if(GLOBAL_WITH_DEBUG)
        message(STATUS "Compiling with debug")
        set(CMAKE_BUILD_TYPE Debug)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_DEBUG")
    else()
        set(CMAKE_BUILD_TYPE Release)
    endif()

    # Enable C++11 support
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(COMPILER_SUPPORTS_CXX0X)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    # Comment this out to disable position independent code
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    # Set the logging level
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLOGGING_LEVEL_1")

endif()


#-------------------------------------------------------------------------------
# Libraries External
##

# Testing
if(GLOBAL_WITH_TESTING AND (NOT HAS_PARENT))
    enable_testing()
    add_subdirectory(${NETLIB_MIDAS_ROOT}/googletest ${NETLIB_BIN_DIR}/external/gtest EXCLUDE_FROM_ALL)
endif()


#-------------------------------------------------------------------------------
# Libraries Internal
##

# Add include directories
set(NETLIB_INCLUDE_DIR ${NETLIB_ROOT}/include)
include_directories(${NETLIB_INCLUDE_DIR})

# Networking library
add_subdirectory(${NETLIB_ROOT}/src ${NETLIB_BIN_DIR}/common)

# Test
add_subdirectory(${NETLIB_ROOT}/test ${NETLIB_BIN_DIR}/test)

