cmake_minimum_required(VERSION 3.21)

project(cringeboy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
include(cmake/CPM.cmake)
include(cmake/suppress-third-party-warnings.cmake)

CPMAddPackage(
  NAME fmt
  GIT_TAG 11.0.2
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL OFF"
  GIT_SHALLOW
)
suppress_third_party_warnings(fmt)

find_package(SDL2 REQUIRED)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_compile_options(-fsanitize=undefined -fsanitize=address)
  add_link_options(-fsanitize=undefined -fsanitize=address)
endif()

add_library(cringeboy_core
  src/core/constants.h
  src/core/emulator.cpp
  src/core/emulator.h
  src/core/memory.cpp
  src/core/memory.h
  src/core/processor.cpp
  src/core/processor.h
  src/core/processor_instr.cpp
  src/core/video.cpp
  src/core/video.h
  src/util.cpp
  src/util.h
)

target_compile_features(cringeboy_core PUBLIC cxx_std_20)
target_include_directories(cringeboy_core PUBLIC src)
target_link_libraries(cringeboy_core PUBLIC SDL2::SDL2 fmt)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(
    cringeboy_core PUBLIC
      -U_FORTIFY_SOURCE
      -D_FORTIFY_SOURCE=3
      -D_GLIBCXX_ASSERTIONS=1
      -fstack-protector-strong
      -fcf-protection=full
      -fstack-clash-protection
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wcast-qual
      -Wformat=2
      -Wundef
      -Werror=float-equal
      -Wshadow
      -Wcast-align
      -Wnull-dereference
      -Wdouble-promotion
      -Wimplicit-fallthrough
      -Wextra-semi
      -Woverloaded-virtual
      -Wnon-virtual-dtor
      -Wold-style-cast
)
endif()

add_executable(cringeboy
  src/main.cpp
  src/sdl_window.h
  src/sdl_window.cpp
)
target_link_libraries(cringeboy PRIVATE cringeboy_core)

if(NOT BUILD_TESTING)
  return()
endif()

target_compile_definitions(cringeboy_core PRIVATE TESTING)

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG v1.15.2
  OPTIONS "BUILD_GMOCK OFF" "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

CPMAddPackage(
  NAME simdjson
  VERSION 3.10.1
  GITHUB_REPOSITORY simdjson/simdjson
  GIT_SHALLOW
)
suppress_third_party_warnings(simdjson)

add_subdirectory(test)
