cmake_minimum_required(VERSION 3.21)

project(gbcxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "" OFF)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME fmt
  GIT_TAG 11.1.3
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL OFF"
  GIT_SHALLOW
  SYSTEM ON
)

CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.15.1
  GITHUB_REPOSITORY gabime/spdlog
  GIT_SHALLOW
  OPTIONS "SPDLOG_INSTALL OFF"
          "SPDLOG_FMT_EXTERNAL ON"
          "SPDLOG_SYSTEM_INCLUDES ON"
          "SPDLOG_NO_EXCEPTIONS ON"
  SYSTEM ON
)

add_subdirectory(3rdparty)

add_library(
  gbcxx_core
  src/core/memory/mbc/mbc0.cpp
  src/core/memory/mbc/mbc0.hpp
  src/core/memory/mbc/mbc1.cpp
  src/core/memory/mbc/mbc1.hpp
  src/core/memory/mbc/mbc3.cpp
  src/core/memory/mbc/mbc3.hpp
  src/core/memory/bus.cpp
  src/core/memory/bus.hpp
  src/core/memory/cartridge.cpp
  src/core/memory/cartridge.hpp
  src/core/sm83/cpu.cpp
  src/core/sm83/cpu.hpp
  src/core/sm83/interrupts.hpp
  src/core/sm83/timer.cpp
  src/core/sm83/timer.hpp
  src/core/video/lcd_control.hpp
  src/core/video/lcd_status.hpp
  src/core/video/ppu.cpp
  src/core/video/ppu.hpp
  src/core/constants.hpp
  src/core/core.cpp
  src/core/core.hpp
  src/core/joypad.hpp
  src/core/util.cpp
  src/core/util.hpp)

target_compile_features(gbcxx_core PUBLIC cxx_std_23)
target_include_directories(gbcxx_core PUBLIC src)
target_link_libraries(gbcxx_core PUBLIC fmt::fmt spdlog::spdlog)

if(CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(gbcxx_core INTERFACE -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(gbcxx_core INTERFACE -fsanitize=undefined -fsanitize=address)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(gbcxx_core INTERFACE
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

add_executable(gbcxx
  src/main.cpp
  src/main_app.hpp
  src/main_app.cpp

  3rdparty/imgui/backends/imgui_impl_sdl3.cpp
  3rdparty/imgui/backends/imgui_impl_sdlrenderer3.cpp
)

# Doesn't build on GNU with -Werror=float-equal
set_source_files_properties(3rdparty/imgui/backends/imgui_impl_sdlrenderer3.cpp PROPERTIES COMPILE_FLAGS -Wno-float-equal)

target_include_directories(gbcxx PRIVATE 3rdparty/imgui_club/imgui_memory_editor)

find_package(SDL3 REQUIRED)
target_link_libraries(gbcxx PRIVATE gbcxx_core SDL3::SDL3 3rdparty::imgui)

add_custom_target(copy_assets ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/assets)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
  DEPENDS ${PROJECT_SOURCE_DIR}/assets
)

if(BUILD_TESTS)
  target_compile_definitions(gbcxx_core PUBLIC -DGBCXX_TESTS)

  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.16.0
    OPTIONS "BUILD_GMOCK OFF"
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt"
    GIT_SHALLOW
    SYSTEM ON
  )

  CPMAddPackage(
    NAME simdjson
    VERSION 3.12.2
    GITHUB_REPOSITORY simdjson/simdjson
    GIT_SHALLOW
    SYSTEM ON
  )

  add_subdirectory(test)
endif()
