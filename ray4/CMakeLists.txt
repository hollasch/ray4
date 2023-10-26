#---------------------------------------------------------------------------------------------------
# CMake Build Configuration for the ray4 4D raytracer
#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10.0...3.27.7)

project (ray4 LANGUAGES C)

# Source
set ( sources_ray4
  src/r4_globals.h
  src/r4_image.h
  src/ray4.h
  src/vector.h
  src/r4_main.c
  src/r4_hit.c
  src/r4_io.c
  src/r4_parse.c
  src/r4_trace.c
  src/r4_vec.c
)

add_executable (ray4 ${sources_ray4})
