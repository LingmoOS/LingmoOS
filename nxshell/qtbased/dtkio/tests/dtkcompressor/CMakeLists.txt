# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkcompressor)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control -fno-inline -Wno-pmf-conversions")

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtkcompressor/*")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/dtkcompressor/archivemanager/*.h"
    "${PROJECT_SOURCE_DIR}/src/dtkcompressor/archivemanager/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/dtkcompressor/3rdparty/libzipplugin/*.cpp"
)
file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp")

include_directories(
    ${PROJECT_SOURCE_DIR}/include/dtkcompressor/
    "../3rdparty/cpp-stub"
    "../3rdparty/stub-ext"
)
add_definitions(-DUSE_FAKE_INTERFACE)
add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
    ${DFAKE_INTERFACE_DBUS_XML}
)

find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(KF5Codecs REQUIRED)
find_package(ZLIB REQUIRED)

target_include_directories(${BIN_NAME} PUBLIC
    ${CMAKE_BINARY_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/dtkcompressor/3rdparty/libzipplugin
    ${PROJECT_SOURCE_DIR}/src/dtkcompressor/archivemanager
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${DtkCore_LIBRARIES}
    KF5::Codecs
    zip
    interfacecompressor
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
