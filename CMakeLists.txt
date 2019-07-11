
cmake_minimum_required(VERSION 3.11.0)

project(ContainerBench
  VERSION 0.1.0.0
  DESCRIPTION "A standard library container benchmark project for Advanced C++ course."
  HOMEPAGE_URL "https://github.com/AronRubin"
)
set(ContainerBench_VENDOR "Aron Rubin")
set(PROJECT_VENDOR "${ContainerBench_VENDOR}")


set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")


if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.8.1
)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.5.0
)

set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "no install of benchmark")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "no testing of benchmark")
set(HAVE_STD_REGEX ON CACHE BOOL "patching an oops in benchmark cmake caching")
# set(BENCHMARK_ENABLE_GTEST_TESTS OFF)
FetchContent_GetProperties(googlebenchmark)
if(NOT googlebenchmark_POPULATED)
  FetchContent_Populate(googlebenchmark)
  add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

enable_testing()

find_package(Threads REQUIRED)

# Add source to this project's executable.
add_executable(ContainerBench src/ContainerBench.cpp)

target_link_libraries(ContainerBench
  PUBLIC Threads::Threads
  PUBLIC benchmark
)

include(FeatureSummary)
feature_summary(WHAT ALL)

# include(DumpProps)
# print_target_properties(diva)
# include(DumpVariables)
# dump_all_variables()



