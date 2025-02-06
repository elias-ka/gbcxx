set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTING OFF)

set_directory_properties(PROPERTIES SYSTEM ON)

add_library(3rdparty_imgui
    imgui/imgui.h
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
)
target_include_directories(3rdparty_imgui INTERFACE imgui/)

add_library(3rdparty::imgui ALIAS 3rdparty_imgui)
