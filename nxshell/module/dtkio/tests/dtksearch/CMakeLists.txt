# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtksearch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtksearch/*")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/dtksearch/*.h"
    "${PROJECT_SOURCE_DIR}/src/dtksearch/*.cpp"
)
file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub/*.cpp"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include/dtksearch/"
    "${PROJECT_SOURCE_DIR}/src/dtksearch/"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub"
)
add_definitions(-DUSE_FAKE_INTERFACE)
add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
    ${DFAKE_INTERFACE_DBUS_XML}
)

find_package(Boost REQUIRED COMPONENTS system)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Lucene REQUIRED IMPORTED_TARGET liblucene++ liblucene++-contrib)
pkg_check_modules(Docparser REQUIRED IMPORTED_TARGET docparser)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    ${Lucene_INCLUDE_DIRS}
    ${Docparser_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${Lucene_LIBRARIES}
    ${Docparser_LIBRARIES}
    ${Boost_LIBRARIES}
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
