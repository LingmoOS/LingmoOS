# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-${LIB_NAME})

find_package(GTest REQUIRED)
file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE TEST_FILES "${CMAKE_CURRENT_LIST_DIR}/*.h" "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_compile_definitions(USE_FAKE_INTERFACE)

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

target_include_directories(${BIN_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/dbus
    ${PROJECT_SOURCE_DIR}/tests/fakeDBus
    ${PROJECT_SOURCE_DIR}/tests/3rdparty
    ${PROJECT_SOURCE_DIR}/tests/mockdemo
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::DBus
    PkgConfig::RSVG2
    GTest::GTest
    Dtk::Core
    pthread
    gcov
    asan
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

target_compile_options(${BIN_NAME} PRIVATE -fno-access-control)

enable_testing()
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
