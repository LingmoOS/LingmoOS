# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkdevice)

find_package(GTest REQUIRED)
file(GLOB_RECURSE HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/*.h" "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_executable(${BIN_NAME}
    ${HEADERS}
    ${SOURCES}
    ${TEST_SOURCES}
)

target_include_directories(${BIN_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}/include/lsdevice"
    "${PROJECT_SOURCE_DIR}/include/global"
    "${PROJECT_SOURCE_DIR}/include/inputdevices"
    "${PROJECT_SOURCE_DIR}/src/3rdparty"
    "${PROJECT_SOURCE_DIR}/src/inputdevices"
    "${PROJECT_SOURCE_DIR}/src/inputdevices/dbus"
    "${PROJECT_SOURCE_DIR}/src/lsdevice"
    "${CMAKE_CURRENT_LIST_DIR}/inputdevices/fakedbus"
    "${CMAKE_CURRENT_LIST_DIR}/3rdparty/cpp-stub"
    "${CMAKE_CURRENT_LIST_DIR}/3rdparty/stub-ext"
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Core
    PkgConfig::LibNl3
    PkgConfig::LibNl3Route
    GTest::GTest
    pthread
    gcov
    asan
)

target_compile_definitions(${BIN_NAME} PRIVATE USE_FAKE_INTERFACE)
target_compile_options(${BIN_NAME} PRIVATE -fsanitize=address)
target_link_options(${BIN_NAME} PRIVATE -fsanitize=address)
target_compile_options(${BIN_NAME} PRIVATE -fno-access-control)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

enable_testing()
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
