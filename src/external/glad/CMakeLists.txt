# Generated from: https://glad.dav1d.de/ on 31 March 2022
cmake_minimum_required (VERSION 3.12)

project("glad")

add_library(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/glad.c)

include_directories(${PROJECT_SOURCE_DIR}/include)