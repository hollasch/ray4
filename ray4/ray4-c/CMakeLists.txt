#---------------------------------------------------------------------------------------------------
# CMake Build Configuration for the ray4 4D raytracer
#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10.0...3.27.7)

set (CMAKE_C_STANDARD          99)
set (CMAKE_C_STANDARD_REQUIRED ON)
set (CMAKE_C_EXTENSIONS        OFF)

project (ray4-c LANGUAGES C)

# Source
set ( sources_ray4_c
  src/ray4.h
  src/r4_image.h
  src/r4_vector.h
  src/r4_hit.c
  src/r4_io.c
  src/r4_main.c
  src/r4_parse.c
  src/r4_trace.c
  src/r4_vec.c
)

add_executable (ray4-c ${sources_ray4_c})

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ray4 m)
endif()
