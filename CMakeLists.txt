cmake_minimum_required(VERSION 3.21)

project(cringeboy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
include(cmake/CPM.cmake)
include(cmake/suppress-third-party-warnings.cmake)

CPMAddPackage(
  NAME expected
  VERSION 1.1.0
  GITHUB_REPOSITORY TartanLlama/expected
  OPTIONS "EXPECTED_BUILD_TESTS OFF"
  GIT_SHALLOW
)
suppress_third_party_warnings(expected)

CPMAddPackage(
  NAME fmt
  GIT_TAG 11.0.2
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL OFF"
  GIT_SHALLOW
)
suppress_third_party_warnings(fmt)

CPMAddPackage(
  NAME simdjson
  VERSION 3.10.1
  GITHUB_REPOSITORY simdjson/simdjson
  GIT_SHALLOW
)
suppress_third_party_warnings(simdjson)

find_package(SDL2 REQUIRED)

add_library(cringeboy_base
  src/common/error.h
  src/common/filesystem.h
  src/common/filesystem.cpp
  src/common/logging.h
  src/common/try.h
  src/core/cpu/flags.h
  src/core/cpu/flags.cpp
  src/core/cpu/processor.h
  src/core/cpu/processor.cpp
  src/core/cpu/registers.h
  src/core/cpu/registers.cpp
  src/core/memory/mmu.h
  src/core/memory/mmu.cpp
  src/core/emulator.h
  src/core/emulator.cpp
  src/sdl_window.h
  src/sdl_window.cpp
)

target_compile_features(cringeboy_base PUBLIC cxx_std_20)
target_include_directories(cringeboy_base PUBLIC src)
target_link_libraries(cringeboy_base PUBLIC tl::expected SDL2::SDL2 fmt)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(
    cringeboy_base
    INTERFACE -Wall
            -Wextra
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Woverloaded-virtual
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
            -Wno-gnu-statement-expression-from-macro-expansion
  )
endif()

add_executable(cringeboy
  src/main.cpp
)
target_link_libraries(cringeboy PRIVATE cringeboy_base)

if(NOT BUILD_TESTING)
  return()
endif()

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG v1.15.2
  OPTIONS "BUILD_GMOCK OFF" "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

add_subdirectory(test)
