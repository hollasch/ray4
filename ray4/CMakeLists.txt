#---------------------------------------------------------------------------------------------------
# CMake Build Configuration for the ray4 4D raytracer
#---------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10.0...4.0.2)

set (CMAKE_CXX_STANDARD          20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS        OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/W4")      # Enable level-4 warnings
    add_compile_options("/wd 4996") # This function or variable may be unsafe. Consider using XXX instead.
endif()

project (ray4 LANGUAGES CXX)

# Source
set ( sources_ray4
  src/ray4.h
  src/r4_color.h
  src/r4_image.h
  src/r4_point.h
  src/r4_ray.h
  src/r4_vector.h
  src/r4_color.cpp
  src/r4_hit.cpp
  src/r4_io.cpp
  src/r4_main.cpp
  src/r4_parse.cpp
  src/r4_point.cpp
  src/r4_ray.cpp
  src/r4_trace.cpp
  src/r4_vector.cpp
)

add_executable (ray4 ${sources_ray4})
add_executable (image4 src/image4.cpp src/r4_image.h)


#---------------------------------------------------------------------------------------------------
# Catch2 Unit Tests
#---------------------------------------------------------------------------------------------------

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.7.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
    src/r4_test.cpp
    src/r4_color.cpp
    src/r4_point.cpp
    src/r4_ray.cpp
    src/r4_vector.cpp
)

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
