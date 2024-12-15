# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-${LIB_NAME})

file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp")

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

add_dependencies(${BIN_NAME} ${LIB_NAME})

target_include_directories(${BIN_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/tests/fakeDBus
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Bluetooth
    ${LIB_NAME}
    Dtk${DTK_VERSION_MAJOR}::Core
    pthread
    gcov
    gtest
    gmock
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
