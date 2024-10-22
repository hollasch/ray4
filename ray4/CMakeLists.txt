#---------------------------------------------------------------------------------------------------
# CMake Build Configuration for the ray4 4D raytracer
#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10.0...3.30.5)

set (CMAKE_CXX_STANDARD          17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS        OFF)

project (ray4 LANGUAGES CXX)

# Source
set ( sources_ray4
  src/r4_globals.h
  src/r4_hit.cpp
  src/r4_image.h
  src/r4_io.cpp
  src/r4_main.cpp
  src/r4_parse.cpp
  src/r4_trace.cpp
  src/r4_vec.cpp
  src/ray4.h
  src/vector.h
)

add_executable (ray4 ${sources_ray4})
