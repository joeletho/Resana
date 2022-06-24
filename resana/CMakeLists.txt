cmake_minimum_required(VERSION 3.21)
project(ResourceAnalyzer)
set(CMAKE_CXX_STANDARD 17)


set(RESANA_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(RESANA_SOURCE_DIR "${RESANA_DIR}/src")
set(RESANA_LIB_DIR "${RESANA_DIR}/lib")
set(RESANA_ASSETS_DIR "${RESANA_DIR}/assets")

set(GLAD_DIR "${RESANA_LIB_DIR}/glad")
set(GLFW_DIR "${RESANA_LIB_DIR}/glfw")
set(IMGUI_DIR "${RESANA_LIB_DIR}/imgui")
set(SPDLOG_DIR "${RESANA_LIB_DIR}/spdlog")


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${GLAD_DIR}")
add_subdirectory("${GLFW_DIR}")
add_subdirectory("${IMGUI_DIR}")
add_subdirectory("${SPDLOG_DIR}")

if (MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif ()

file(GLOB_RECURSE RESANA_SOURCES
        "${RESANA_SOURCE_DIR}/*.h"
        "${RESANA_SOURCE_DIR}/*.cpp"
        )

list(REMOVE_ITEM RESANA_SOURCES
        "${RESANA_SOURCE_DIR}/rspch.h" # Will be precompiled
        )

message(${RESANA_SOURCES})

add_executable(${PROJECT_NAME} "${RESANA_SOURCES}")

target_precompile_headers(${PROJECT_NAME} PRIVATE "${RESANA_SOURCE_DIR}/rspch.h")

target_include_directories(${PROJECT_NAME} PUBLIC
        "${RESANA_DIR}"
        "${RESANA_SOURCE_DIR}"

        "${GLAD_DIR}"
        "${GLFW_DIR}"
        "${IMGUI_DIR}"
        "${SPDLOG_DIR}"
        )

target_link_libraries(${PROJECT_NAME} PUBLIC
        "glad"
        "glfw"
        "imgui"
        "spdlog"
        "pdh" # pdh.lib for Windows Pdh.h functions
        )

# -------------------------------------------------------------------
# Copy executable dependencies to CMake runtime output directory
# -------------------------------------------------------------------

add_custom_target(copyAssets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESANA_ASSETS_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
        )

add_dependencies(${PROJECT_NAME} copyAssets)
# -----------------------------------------------------------------

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE=1)
target_compile_definitions(${PROJECT_NAME} PUBLIC RS_ENABLE_ASSERTS=1 RS_DEBUG=1 RS_BUILD_DLL=1 BUILD_SHARED_LIB=1)
