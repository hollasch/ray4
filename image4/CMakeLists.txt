#---------------------------------------------------------------------------------------------------
# CMake Build Configuration for The Image4 4D Image Tool
#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10.0...3.30.5)

set (CMAKE_CXX_STANDARD          20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS        OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/W4")      # Enable level-4 warnings
    add_compile_options("/wd 4996") # This function or variable may be unsafe. Consider using XXX instead.
endif()

project (image4 LANGUAGES CXX)

add_executable (image4 image4.cpp)
