cmake_minimum_required(VERSION 3.21)
project(ResAna)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(RESANA_DIR "${CMAKE_CURRENT_LIST_DIR}/resana")

include_directories(
        "${RESANA_DIR}/src"
)

add_subdirectory("${RESANA_DIR}")
