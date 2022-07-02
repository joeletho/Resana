cmake_minimum_required(VERSION 3.19)
project(imgui)

set(IMGUI_SOURCES
        "imconfig.h"
        "imgui.h"
        "imgui.cpp"
        "imstb_rectpack.h"
        "imstb_truetype.h"
        "imgui_internal.h"
        "imstb_textedit.h"
        "imgui_draw.cpp"
        "imgui_demo.cpp"
        "imgui_tables.cpp"
        "imgui_widgets.cpp"
        "backends/imgui_impl_glfw.h"
        "backends/imgui_impl_glfw.cpp"
        "backends/imgui_impl_opengl3.h"
        "backends/imgui_impl_opengl3.cpp"
        "backends/imgui_impl_opengl3_loader.h"
        )

add_library(imgui STATIC "${IMGUI_SOURCES}")

target_precompile_headers(imgui PUBLIC "imgui.h")

target_include_directories(imgui PUBLIC
        "./"
        "backends"
        )

# glfw
target_link_libraries(imgui PUBLIC glfw)
