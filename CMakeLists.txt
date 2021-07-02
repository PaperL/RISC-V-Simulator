cmake_minimum_required(VERSION 3.16)
project(RISC-V-Simulator)
add_compile_options(-std=c++2a -fconcepts)

include_directories(
        ${PROJECT_SOURCE_DIR}/src/
)
set(src_dir
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${PROJECT_SOURCE_DIR}/src/stage.cpp

        ${PROJECT_SOURCE_DIR}/src/cpu.hpp
        ${PROJECT_SOURCE_DIR}/src/predictor.hpp
        )
add_executable(code ${src_dir})