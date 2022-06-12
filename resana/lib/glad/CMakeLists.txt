cmake_minimum_required(VERSION 3.16)
project(glad)

set(GLAD_SOURCES
        "src/glad.c"
        "include/glad/glad.h"
        "include/KHR/khrplatform.h"
        )

add_library(glad STATIC "${GLAD_SOURCES}")

target_precompile_headers(glad PUBLIC "include/glad/glad.h")

target_include_directories(glad PUBLIC
        "src"
        "include"
        )

