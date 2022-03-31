cmake_minimum_required (VERSION 3.12)

project("rPBR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -W -Wno-deprecated-declarations -Wno-missing-field-initializers -O0 -g -march=native")

option(DEBUG_MODE "More debug information" ON)
if (DEBUG_MODE)
    add_definitions(-DEBUG_MODE)
endif()
option(BUILD_ALL_EXAMPLES "Buils all libraries examples" OFF)

find_package(glfw3 3.3 REQUIRED)

add_subdirectory(src/external/raylib)
add_subdirectory(src/external/glad)


include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/external
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable (rpbr ${CMAKE_CURRENT_SOURCE_DIR}/src/rpbr.cpp)
target_link_libraries(rpbr glfw glad raylib)