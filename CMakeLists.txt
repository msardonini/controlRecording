cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

project(remoteSender)

option(GLOBAL_FOR_RPI "GLOBAL_FOR_RPI" OFF)


# Define the compiler
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()


if(GLOBAL_FOR_RPI)
    message(STATUS "Compiling for Raspberry Pi")
    set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/lib/x-tools/arm-unknown-linux-gnueabihf/bin/arm-unknown-linux-gnueabihf-g++)
    set(CMAKE_C_COMPILER /lib/x-tools/arm-unknown-linux-gnueabihf/bin/arm-unknown-linux-gnueabihf-gcc)
endif()


include_directories(inc)

add_subdirectory(lib/net-lib ${CMAKE_CURRENT_SOURCE_DIR}/build/net-lib)
include_directories(lib/net-lib/include)
link_directories(lib/net-lib/build/common)



add_executable(remoteSender
		src/remoteSender/remoteSenderApp.cpp
		src/remoteSender/remoteSender.cpp)

target_link_libraries(remoteSender
		NetLib
		wiringPi)