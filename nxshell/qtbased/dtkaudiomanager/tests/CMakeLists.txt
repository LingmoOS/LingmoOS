# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkaudiomanager)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

add_definitions(-DUSE_FAKE_INTERFACE)

file(GLOB_RECURSE INCLUDE_FILES "../include/*.h")
file(GLOB_RECURSE SRCS
    "../src/*.h"
    "../src/*.cpp"
    "../dde-daemon-plugin/*.h"
    "../dde-daemon-plugin/*.cpp"
    "./fakedbus/daemonaudiomanagerservice.h"
    "./fakedbus/daemonaudiomanagerservice.cpp"
)
file(GLOB TEST_FILES
    "./main.cpp"
    "./ut-daudiohelpers.hpp"
    "./ut-daudioport.cpp"
    "./ut-daudiocard.cpp"
    "./ut-daudiodevice.cpp"
    "./ut-daudiostream.cpp"
    "./ut-daudiomanager.cpp"
    "./ut-ddaemonaudiomanager.cpp"
)

include_directories(
    "../include"
    "../dde-daemon-plugin"
    "../src"
)

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Test
    Dtk${DTK_VERSION_MAJOR}::Core
    -lpthread
    -lgcov
    -lgtest
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

enable_testing()
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
