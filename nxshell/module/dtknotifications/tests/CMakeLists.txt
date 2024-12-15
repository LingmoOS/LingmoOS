# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(CMAKE_PROJECT_NAME ut-${CMAKE_PROJECT_NAME})

find_package(GTest REQUIRED)
file(GLOB_RECURSE INCLUDE_FILES "../include/*.h")
file(GLOB_RECURSE SRCS
    "../src/*.h"
    "../src/*.cpp"
)

file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp")

include_directories(
    "../include"
    "../src/dbus"
)

add_definitions(-DUSE_FAKE_INTERFACE)

add_executable(${CMAKE_PROJECT_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkCore_INCLUDE_DIRS}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkCore_LIBRARIES}
    -lpthread
    -lgcov
    -lgtest
    -lcrypt
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage -fno-access-control)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage -fno-access-control)
endif()

enable_testing()
add_test(NAME ${CMAKE_PROJECT_NAME} COMMAND ${CMAKE_PROJECT_NAME})
