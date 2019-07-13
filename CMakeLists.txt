cmake_minimum_required(VERSION 3.11.0)
project(ContainerBench)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "no install of benchmark")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "no testing of benchmark")
set(HAVE_STD_REGEX ON CACHE BOOL "patching an oops in benchmark cmake caching")
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.5.0
)
FetchContent_GetProperties(googlebenchmark)
if(NOT googlebenchmark_POPULATED)
  FetchContent_Populate(googlebenchmark)
  add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(ContainerBench src/ContainerBench.cpp)
target_link_libraries(ContainerBench PUBLIC benchmark)
