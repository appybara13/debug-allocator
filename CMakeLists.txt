cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED on)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

option(DALLOC_BUILD_TESTS "Build dalloc tests" off)

project(
  dalloc
  VERSION 0.1.0
  LANGUAGES C)

add_library(dalloc src/dalloc.c)
target_include_directories(dalloc PUBLIC inc/)

if(DALLOC_BUILD_TESTS)

  add_executable(dalloc_tests tests/tests.c)
  target_link_libraries(dalloc_tests PRIVATE dalloc)

endif()
