cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# "../" --> output directory relative same to running in IDE
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
 
project(miragedemo)

find_package(OpenGL REQUIRED)

include_directories(./src)

add_executable (miragedemo WIN32 ./src/demo.cpp ./src/stb_image.h fotoalbum.rc)
