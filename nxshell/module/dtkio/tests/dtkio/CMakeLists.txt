# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkio)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtkio/*")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/dtkio/*.h"
    "${PROJECT_SOURCE_DIR}/src/dtkio/*.cpp"
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
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIOUNIX REQUIRED gio-unix-2.0)

target_include_directories(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${GLIB_INCLUDE_DIRS}
    ${GIOUNIX_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/dtkio
    ${PROJECT_SOURCE_DIR}/include/dtkio
    ${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext
    ${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Dtk::Core
    ${GLIB_LIBRARIES}
    ${GIOUNIX_LIBRARIES}
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
