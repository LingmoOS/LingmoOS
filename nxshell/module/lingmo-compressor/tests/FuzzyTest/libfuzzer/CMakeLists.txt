# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.0.0)

project(test_libfuzzer VERSION 0.1 LANGUAGES CXX)

set( CMAKE_CXX_FLAGS "-std=c++11" )
set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address,leak -fsanitize=fuzzer")
set(CMAKE_C_FLAGS " ${CMAKE_C_FLAGS} -g -fsanitize=undefined,address,leak -fsanitize=fuzzer")

include_directories(${CMAKE_SOURCE_DIR}/../../../3rdparty/ChardetDetector/)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/) 

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core)
file(GLOB_RECURSE
  c_files
  ../../../3rdparty/ChardetDetector/*.cpp
  ../../../3rdparty/ChardetDetector/tables/*.cpp
  )

add_executable(${PROJECT_NAME} "main.cpp"  ${c_files})

target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libFuzzer.a)

execute_process(COMMAND /usr/bin/cp -rf ${CMAKE_SOURCE_DIR}/libfuzzer_in ${PROJECT_BINARY_DIR}/)
execute_process(COMMAND mkdir -p ${PROJECT_BINARY_DIR}/libfuzzer_out)
