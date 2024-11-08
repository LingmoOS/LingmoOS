cmake_minimum_required(VERSION 3.5)

project(testburiedpoint LANGUAGES C)

set(BURIEDPOINT_TEST_TOP_DIR ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -g)

find_package(PkgConfig REQUIRED)

pkg_check_modules(DIAGNOSTICS lingmosdk-diagnostics)
target_include_directories(${PROJECT_NAME} PRIVATE ${DIAGNOSTICS_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${DIAGNOSTICS_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DIAGNOSTICS_LIBRARIES})

set(SRCS 
    "${BURIEDPOINT_TEST_TOP_DIR}/main.c")

target_include_directories(${PROJECT_NAME} PRIVATE ${BURIEDPOINT_TEST_TOP_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${SRCS})