cmake_minimum_required(VERSION 3.21)

project(gbcxx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "" OFF)

include(cmake/CPM.cmake)

cpmaddpackage(
  NAME
  fmt
  GIT_TAG
  11.1.4
  GITHUB_REPOSITORY
  fmtlib/fmt
  OPTIONS
  "FMT_INSTALL OFF"
  GIT_SHALLOW
  SYSTEM
  ON)

cpmaddpackage(
  NAME
  spdlog
  GIT_TAG
  v1.15.2
  GITHUB_REPOSITORY
  gabime/spdlog
  GIT_SHALLOW
  OPTIONS
  "SPDLOG_INSTALL OFF"
  "SPDLOG_FMT_EXTERNAL ON"
  "SPDLOG_SYSTEM_INCLUDES ON"
  "SPDLOG_NO_EXCEPTIONS ON"
  "SPDLOG_BUILD_SHARED OFF"
  SYSTEM
  ON)

cpmaddpackage(
  NAME
  SDL
  VERSION
  3.2.10
  GIT_TAG
  release-3.2.10
  GITHUB_REPOSITORY
  libsdl-org/SDL
  GIT_SHALLOW
  OPTIONS
  "SDL_X11 ON"
  "SDL_WAYLAND ON"
  "SDL_CCACHE ON"
  "SDL_STATIC ON"
  "SDL_SHARED OFF"
  "SDL_BUILD_SHARED_LIBS OFF"
  "SDL_DISABLE_INSTALL ON"
  "SDL_TEST_LIBRARY OFF"
  SYSTEM
  ON)

add_library(
  gbcxx_core OBJECT
  src/core/memory/mbc.cpp
  src/core/memory/mbc.hpp
  src/core/memory/bus.cpp
  src/core/memory/bus.hpp
  src/core/memory/cartridge.cpp
  src/core/memory/cartridge.hpp
  src/core/sm83/cpu.cpp
  src/core/sm83/cpu.hpp
  src/core/sm83/interrupts.hpp
  src/core/sm83/timer.cpp
  src/core/sm83/timer.hpp
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

if(CMAKE_BUILD_TYPE MATCHES "Debug" AND CMAKE_CXX_COMPILER_ID MATCHES
                                        "Clang|GNU")
  target_compile_options(
    gbcxx_core INTERFACE -fsanitize=undefined -fsanitize=address
                         -fno-omit-frame-pointer)
  target_link_options(gbcxx_core INTERFACE -fsanitize=undefined
                      -fsanitize=address)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  if(NOT DEFINED EMSCRIPTEN)
    target_compile_options(
      gbcxx_core
      PUBLIC -D_GLIBCXX_ASSERTIONS=1
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

  target_link_options(gbcxx_core INTERFACE -static-libgcc -static-libstdc++)
endif()

add_executable(gbcxx src/main.cpp src/main_app.hpp src/main_app.cpp)

target_link_libraries(gbcxx PRIVATE gbcxx_core SDL3::SDL3-static)

if(EMSCRIPTEN)
  target_link_options(
    gbcxx
    PRIVATE
    -sSTACK_OVERFLOW_CHECK=1
    -sWASM=1
    -sASSERTIONS
    -sSAFE_HEAP=1
    -sSTACK_SIZE=8MB
    -sALLOW_MEMORY_GROWTH
    -sFORCE_FILESYSTEM
    -sEXPORTED_RUNTIME_METHODS=['callMain']
    -sINVOKE_RUN=0
    --emrun
    --shell-file
    "./emscripten_shell.html")
  set_target_properties(gbcxx PROPERTIES OUTPUT_NAME "index")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")

  add_custom_target(copy-emscripten-shell ALL
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/emscripten_shell.html)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/emscripten_shell.html PRE_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/emscripten_shell.html
      ${CMAKE_CURRENT_BINARY_DIR}/emscripten_shell.html
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/emscripten_shell.html)
endif()

if(BUILD_TESTS)
  target_compile_definitions(gbcxx_core PUBLIC -DGBCXX_TESTS)

  cpmaddpackage(
    NAME
    googletest
    GITHUB_REPOSITORY
    google/googletest
    GIT_TAG
    v1.16.0
    OPTIONS
    "BUILD_GMOCK OFF"
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt"
    GIT_SHALLOW
    SYSTEM
    ON)

  cpmaddpackage(
    NAME
    simdjson
    VERSION
    3.12.2
    GITHUB_REPOSITORY
    simdjson/simdjson
    GIT_SHALLOW
    SYSTEM
    ON)

  add_subdirectory(test)
endif()
