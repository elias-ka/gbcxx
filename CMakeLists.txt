cmake_minimum_required(VERSION 3.21)

project(gbcxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME fmt
  GIT_TAG 11.0.2
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL OFF"
  GIT_SHALLOW
  SYSTEM ON
)

CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.15.0
  GITHUB_REPOSITORY gabime/spdlog
  GIT_SHALLOW
  OPTIONS "SPDLOG_INSTALL OFF"
          "SPDLOG_FMT_EXTERNAL ON"
          "SPDLOG_SYSTEM_INCLUES ON"
          "SPDLOG_NO_EXCEPTIONS ON"
  SYSTEM ON
)

CPMAddPackage(
  NAME SDL
  VERSION 3.2.0
  GIT_TAG release-3.2.0
  GITHUB_REPOSITORY libsdl-org/SDL
  GIT_SHALLOW
  OPTIONS "BUILD_SHARED_LIBS OFF"
          "SDL_X11 OFF"
          "SDL_CCACHE ON"
  SYSTEM ON
)

if(CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=undefined -fsanitize=address)
endif()

add_subdirectory(3rdparty)

add_library(
  gbcxx_core
  src/core/mbc/mbc0.cpp
  src/core/mbc/mbc0.hpp
  src/core/mbc/mbc1.cpp
  src/core/mbc/mbc1.hpp
  src/core/constants.hpp
  src/core/core.cpp
  src/core/core.hpp
  src/core/mbc.cpp
  src/core/mbc.hpp
  src/core/ppu.cpp
  src/core/ppu.hpp
  src/core/cpu.cpp
  src/core/cpu.hpp
  src/core/timer.cpp
  src/core/timer.hpp
  src/core/util.cpp
  src/core/util.hpp)

target_compile_features(gbcxx_core PUBLIC cxx_std_23)
target_include_directories(gbcxx_core PUBLIC src)
target_link_libraries(gbcxx_core PUBLIC fmt::fmt spdlog::spdlog)

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

target_include_directories(gbcxx PRIVATE 3rdparty/imgui_club/imgui_memory_editor)
target_link_libraries(gbcxx PRIVATE gbcxx_core SDL3::SDL3 3rdparty::imgui)

add_custom_target(copy_assets ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/assets)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
  DEPENDS ${PROJECT_SOURCE_DIR}/assets
)

if(BUILD_TESTS)
  include(CTest)
  enable_testing()
  target_compile_definitions(gbcxx_core PRIVATE GBCXX_TESTS)

  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG v1.15.2
    OPTIONS "BUILD_GMOCK OFF"
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt"
    SYSTEM ON
  )

  CPMAddPackage(
    NAME simdjson
    VERSION 3.11.3
    GITHUB_REPOSITORY simdjson/simdjson
    GIT_SHALLOW
    SYSTEM ON
  )

  add_subdirectory(test)
endif()
