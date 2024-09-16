# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

PROJECT(dde-notificationwidget-plugin)
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(REQUIRED_QT_VERSION 5.11.3)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets DBus Concurrent Svg X11Extras)
find_package(DtkCore REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkWidget REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

set(XML2CPP_DIR ${PROJECT_SOURCE_DIR}/utils/dbus/xml2cpp)

#dbus xml
find_package(DtkTools REQUIRED)

function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND ${DTK_XML2CPP} -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${XML2CPP_DIR})
endfunction(generation_dbus_interface)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/xml/org.deepin.dde.daemon.Launcher1.xml
    Launcher
    ${XML2CPP_DIR}/launcher_interface
)
file(GLOB DBUS_TYPES "utils/dbus/xml2cpp/types/*.*")
file(GLOB DBUS_INTERFACES "utils/dbus/xml2cpp/*.*")

include(src.cmake)

SET(LIBRARY_OUTPUT_PATH ../app/plugins)
add_library(${PROJECT_NAME} MODULE
    plugin.h
    plugin.cpp
    ${SOURCES}
    ${HEADERS}
    image.qrc
    ${DBUS_TYPES}
    ${DBUS_INTERFACES}
)

set(COMMON_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
    Qt5::Svg
    Qt5::X11Extras
    ${DtkWidget_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${COMMON_LIBS})

target_include_directories(${PROJECT_NAME} PRIVATE
    Qt5::Concurrent
    Qt5::Svg
    Qt5::X11Extras
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${DtkWidget_INCLUDE_DIRS}
    utils/dbus/xml2cpp/
    utils/dbus/xml2cpp/types/
)
include_directories(
    ../interface/
)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/dde-widgets/plugins)
