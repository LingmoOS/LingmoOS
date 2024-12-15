# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkburn)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtkburn/*")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/dtkburn/*.h"
    "${PROJECT_SOURCE_DIR}/src/dtkburn/*.cpp"
)
file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub/*.cpp"
    )

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ISOBURN REQUIRED libisoburn-1)

target_include_directories(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${ISOBURN_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/dtkburn
    ${PROJECT_SOURCE_DIR}/include/dtkburn
    ${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext
    ${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Dtk::Core
    ${ISOBURN_LIBRARIES}
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
