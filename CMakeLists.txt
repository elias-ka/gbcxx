cmake_minimum_required(VERSION 3.21)

project(gbcxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
include(cmake/CPM.cmake)
include(cmake/suppress-third-party-warnings.cmake)

cpmaddpackage(
  NAME
  fmt
  GIT_TAG
  11.0.2
  GITHUB_REPOSITORY
  fmtlib/fmt
  OPTIONS
  "FMT_INSTALL OFF"
  GIT_SHALLOW)
suppress_third_party_warnings(fmt)

cpmaddpackage(
  NAME
  spdlog
  GIT_TAG
  v1.15.0
  GITHUB_REPOSITORY
  gabime/spdlog
  OPTIONS
  "SPDLOG_INSTALL OFF"
  "SPDLOG_FMT_EXTERNAL ON"
  "SPDLOG_SYSTEM_INCLUES ON"
  "SPDLOG_NO_EXCEPTIONS ON"
  GIT_SHALLOW)

find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
  add_compile_options(-fsanitize=undefined -fsanitize=address)
  add_link_options(-fsanitize=undefined -fsanitize=address)
endif()

add_library(
  gbcxx_core
  src/core/mbc/mbc0.cpp
  src/core/mbc/mbc0.hpp
  src/core/mbc/mbc1.cpp
  src/core/mbc/mbc1.hpp
  src/core/constants.hpp
  src/core/emulator.cpp
  src/core/emulator.hpp
  src/core/mbc.cpp
  src/core/mbc.hpp
  src/core/memory.cpp
  src/core/memory.hpp
  src/core/ppu.cpp
  src/core/ppu.hpp
  src/core/processor.cpp
  src/core/processor.hpp
  src/core/processor_instr.cpp
  src/core/timer.cpp
  src/core/timer.hpp
  src/sdl_window.hpp
  src/sdl_window.cpp
  src/util.cpp
  src/util.hpp)

target_compile_features(gbcxx_core PUBLIC cxx_std_23)
target_include_directories(gbcxx_core PUBLIC src)
target_link_libraries(gbcxx_core PUBLIC SDL2::SDL2 fmt::fmt spdlog::spdlog)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(
    gbcxx_core
    PUBLIC -D_GLIBCXX_ASSERTIONS=1
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
           -Wold-style-cast)
endif()

add_executable(gbcxx src/main.cpp)
target_link_libraries(gbcxx PRIVATE gbcxx_core)

if(NOT BUILD_TESTING)
  return()
endif()

target_compile_definitions(gbcxx_core PRIVATE GBCXX_TESTING)

cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  google/googletest
  GIT_TAG
  v1.15.2
  OPTIONS
  "BUILD_GMOCK OFF"
  "INSTALL_GTEST OFF"
  "gtest_force_shared_crt")

cpmaddpackage(
  NAME
  simdjson
  VERSION
  3.11.3
  GITHUB_REPOSITORY
  simdjson/simdjson
  GIT_SHALLOW)
suppress_third_party_warnings(simdjson)

add_subdirectory(test)
