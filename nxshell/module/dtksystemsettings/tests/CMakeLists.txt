# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(TEST_NAME ut-${LIB_NAME})

add_executable(${TEST_NAME})

file(GLOB_RECURSE TEST_SOURCES
    "./*.h"
    "./*.cpp"
    "./*.qrc"
)

file(GLOB_RECURSE LIB_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

find_package(GTest REQUIRED)

target_sources(${TEST_NAME} PRIVATE
    ${LIB_PUBLIC_HEADERS}
    ${LIB_SOURCES}
    ${TEST_SOURCES}
    "main.cpp"
)

file(GLOB DBUS_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/src/*/dbus")
file(GLOB FAKEDBUS_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/tests/*/fakedbus")


target_include_directories(${TEST_NAME} PRIVATE
    ${DBUS_INCLUDE_DIRS}
    ${FAKEDBUS_INCLUDE_DIRS}
)

target_include_directories(${TEST_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/accounts
    ${PROJECT_SOURCE_DIR}/include/login
    ${PROJECT_SOURCE_DIR}/include/power
    ${PROJECT_SOURCE_DIR}/include/rfmanager
    ${PROJECT_SOURCE_DIR}/include/systemtime
    ${PROJECT_SOURCE_DIR}/src/accounts
    ${PROJECT_SOURCE_DIR}/src/login
    ${PROJECT_SOURCE_DIR}/src/power
    ${PROJECT_SOURCE_DIR}/src/rfmanager
    ${PROJECT_SOURCE_DIR}/src/systemtime
)

target_link_libraries(${TEST_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkCore_LIBRARIES}
    crypt
    pthread
    gcov
    gtest
    gmock
)

target_compile_definitions(${TEST_NAME} PRIVATE USE_FAKE_INTERFACE)
target_compile_options(${TEST_NAME} PRIVATE -fno-access-control)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
target_compile_options(${TEST_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
target_compile_options(${TEST_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

enable_testing()
add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
