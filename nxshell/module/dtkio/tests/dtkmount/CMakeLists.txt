# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME ut-dtkmount)

find_package(GTest REQUIRED)
file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/dtkmount/*")
file(GLOB_RECURSE SRCS
    "${PROJECT_SOURCE_DIR}/src/dtkmount/*.h"
    "${PROJECT_SOURCE_DIR}/src/dtkmount/*.cpp"
)
file(GLOB_RECURSE TEST_FILES "./*.h" "./*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext/*.cpp"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub/*.cpp"
    )

# using classes generated from D-Bus XML
set(UDISKS2_OBJECT_XML ${PROJECT_SOURCE_DIR}/misc/dtkmount/org.freedesktop.UDisks2.ObjectManager.xml)
set(UDISKS2_XML ${PROJECT_SOURCE_DIR}/misc/dtkmount/org.freedesktop.UDisks2.xml)
set_source_files_properties(
    ${UDISKS2_XML}
    PROPERTIES NO_NAMESPACE true INCLUDE ${PROJECT_SOURCE_DIR}/src/dtkmount/dbus/udisks2_dbus_common.h
    )
if(QT_VERSION_MAJOR EQUAL "6")
    qt_add_dbus_interface(SRCS ${UDISKS2_OBJECT_XML} objectmanager_interface)
    qt_add_dbus_interface(SRCS ${UDISKS2_XML} udisks2_interface)
else()
    qt5_add_dbus_interface(SRCS ${UDISKS2_OBJECT_XML} objectmanager_interface)
    qt5_add_dbus_interface(SRCS ${UDISKS2_XML} udisks2_interface)
endif()

include_directories(
    "${PROJECT_SOURCE_DIR}/include/dtkmount/"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/stub-ext"
    "${PROJECT_SOURCE_DIR}/tests/3rdparty/cpp-stub"
)

add_executable(${BIN_NAME}
    ${INCLUDE_FILES}
    ${SRCS}
    ${TEST_FILES}
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GIOUNIX REQUIRED gio-unix-2.0)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

target_include_directories(${BIN_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${GLIB_INCLUDE_DIRS}
    ${GIOUNIX_INCLUDE_DIRS}
    ${LIBSECRET_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk::Core
    ${GLIB_LIBRARIES}
    ${GIOUNIX_LIBRARIES}
    ${LIBSECRET_LIBRARIES}
    -lpthread
    -lgcov
    -lgtest
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline -fno-access-control")


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

enable_testing()
add_test(NAME ${BIN_NAME} COMMAND ${BIN_NAME})
