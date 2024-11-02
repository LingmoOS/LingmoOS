cmake_minimum_required(VERSION 3.5)

project(testlockscreen LANGUAGES C)

set(TESTLOCKSCREEN_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

find_package(PkgConfig REQUIRED)
pkg_check_modules(KYPOWERMANAGEMENT lingmosdk-powermanagement)
target_include_directories(${PROJECT_NAME} PRIVATE ${KYPOWERMANAGEMENT_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${KYPOWERMANAGEMENT_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${KYPOWERMANAGEMENT_LIBRARIES})

set(SRCS 
    "${TESTLOCKSCREEN_TOP_DIR}/main.c")

target_sources(${PROJECT_NAME} PRIVATE ${SRCS})
