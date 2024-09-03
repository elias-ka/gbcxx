cmake_minimum_required(VERSION 3.21)

project(cringeboy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CPM.cmake)
CPMAddPackage("gh:TartanLlama/expected@1.1.0")
CPMAddPackage("gh:fmtlib/fmt#11.0.2")

find_package(SDL2 REQUIRED)

add_executable(cringeboy
  src/common/error.h
  src/common/filesystem.h
  src/common/filesystem.cpp
  src/common/logging.h
  src/common/try.h
  src/core/cpu/processor.h
  src/core/cpu/processor.cpp
  src/core/memory/mmu.h
  src/core/memory/mmu.cpp
  src/emulator.h
  src/emulator.cpp
  src/sdl_window.h
  src/sdl_window.cpp
  src/main.cpp
)

target_compile_features(cringeboy PRIVATE cxx_std_20)
target_include_directories(cringeboy PRIVATE src)
target_link_libraries(cringeboy PRIVATE tl::expected SDL2::SDL2 fmt)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(
    cringeboy
    PRIVATE -Wall
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
