# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(LIB_NAME ut-${LIB_NAME})

file(GLOB_RECURSE INCLUDE_FILES
    "../include/*.h"
)

file(GLOB_RECURSE SRCS
    "../src/*.h"
    "../src/*.cpp"
)

file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp")

add_compile_definitions(USE_FAKE_INTERFACE)

add_executable(${LIB_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

target_include_directories(${LIB_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/settings
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/dbus
    ${PROJECT_SOURCE_DIR}/tests/fakeDBus
)

target_link_libraries(${LIB_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Dtk${DTK_VERSION_MAJOR}::Core
    PkgConfig::Gio
    PkgConfig::NetworkManager
    pthread
    gcov
    gtest
    gmock
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${LIB_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${LIB_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

target_compile_options(${LIB_NAME} PRIVATE -fno-access-control)

enable_testing()
add_test(NAME ${LIB_NAME} COMMAND ${LIB_NAME})
