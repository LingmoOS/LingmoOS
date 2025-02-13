cmake_minimum_required(VERSION 3.10)
project(lingmo-ocr)

#3rdparty
execute_process(COMMAND sh ${CMAKE_CURRENT_LIST_DIR}/build3rdparty.sh WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

option(DOTEST "option for test" OFF)

if(DOTEST)
    add_definitions(-DIN_TEST)
endif()

#lingmo-ocr
add_subdirectory(src)
