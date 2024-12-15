# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME dtkmount)

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/*")
file(GLOB_RECURSE SRCS
    "./*.h"
    "./*.cpp"
)

# using classes generated from D-Bus XML
set(UDISKS2_OBJECT_XML ${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/org.freedesktop.UDisks2.ObjectManager.xml)
set(UDISKS2_XML ${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/org.freedesktop.UDisks2.xml)
set_source_files_properties(
    ${UDISKS2_XML}
    PROPERTIES NO_NAMESPACE true INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/dbus/udisks2_dbus_common.h
    )
if(QT_VERSION_MAJOR EQUAL "6")
    qt_add_dbus_interface(SRCS ${UDISKS2_OBJECT_XML} objectmanager_interface)
    qt_add_dbus_interface(SRCS ${UDISKS2_XML} udisks2_interface)
else()
    qt5_add_dbus_interface(SRCS ${UDISKS2_OBJECT_XML} objectmanager_interface)
    qt5_add_dbus_interface(SRCS ${UDISKS2_XML} udisks2_interface)
endif()

add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${SRCS}
)

target_include_directories(${BIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/)

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

find_package(DtkCore REQUIRED)
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
)

install(FILES ${INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# config qmake moudule file
set(DTK_MODULE ${BIN_NAME})
set(DTK_DEPS "core dbus")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/qt_lib_${BIN_NAME}.pri.in qt_lib_${BIN_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${BIN_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")
