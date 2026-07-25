cmake_minimum_required(VERSION 3.15)

project(qhotkey
    VERSION 1.5.0
    DESCRIPTION "Global hotkey library for Qt software"
    HOMEPAGE_URL "https://skycoder42.github.io/QHotkey/"
    LANGUAGES CXX)

option(QHOTKEY_EXAMPLES "Build examples" OFF)
option(QHOTKEY_INSTALL "Enable install rule" ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT QT_DEFAULT_MAJOR_VERSION)
    set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "Qt version to use (5 or 6), defaults to 5")
endif()

set(KF6_MIN_VERSION "6.5.0")

find_package(ECM ${KF6_MIN_VERSION}  NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(KDEInstallDirs)

find_package(Qt${QT_DEFAULT_MAJOR_VERSION} 6.2.0 COMPONENTS Core Gui REQUIRED)

add_library(qhotkey QHotkey/qhotkey.cpp)
add_library(QHotkey::QHotkey ALIAS qhotkey)
target_link_libraries(qhotkey PUBLIC Qt${QT_DEFAULT_MAJOR_VERSION}::Core Qt${QT_DEFAULT_MAJOR_VERSION}::Gui)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(qhotkey PRIVATE QHOTKEY_LIBRARY)
    target_compile_definitions(qhotkey PUBLIC QHOTKEY_SHARED)
endif()

if(APPLE)
    find_library(CARBON_LIBRARY Carbon)
    mark_as_advanced(CARBON_LIBRARY)

    target_sources(qhotkey PRIVATE QHotkey/qhotkey_mac.cpp)
    target_link_libraries(qhotkey PRIVATE ${CARBON_LIBRARY})
elseif(WIN32)
    target_sources(qhotkey PRIVATE QHotkey/qhotkey_win.cpp)
else()
    find_package(X11 REQUIRED)
    include_directories(${X11_INCLUDE_DIR})

    find_package(KF6 ${KF6_MIN_VERSION}  COMPONENTS
        GlobalAccel
        WindowSystem
    )

    target_link_libraries(qhotkey PRIVATE 
        ${X11_LIBRARIES}
        Qt${QT_DEFAULT_MAJOR_VERSION}::GuiPrivate
        KF6::GlobalAccel
        KF6::WindowSystem
    )
    target_sources(qhotkey PRIVATE QHotkey/qhotkey_linux.cpp QHotkey/xdgshortcut.cpp)

    set(kglobalaccel_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf6_org.kde.KGlobalAccel.xml)
    message(STATUS "kglobalaccel_xml: ${kglobalaccel_xml}")
    set_source_files_properties(${kglobalaccel_xml} PROPERTIES
        INCLUDE "kglobalaccel.h"
        NO_NAMESPACE TRUE
        CLASSNAME "KGlobalAccelInterface"
    )
    qt_add_dbus_interface(kglobalaccel_dbus_SRC ${kglobalaccel_xml} kglobalaccel_interface)

    set(kglobalaccel_component_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf6_org.kde.kglobalaccel.Component.xml)
    set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES
        INCLUDE "kglobalshortcutinfo.h"
        NO_NAMESPACE TRUE
        CLASSNAME "KGlobalAccelComponentInterface"
    )
    qt_add_dbus_interface(kglobalaccel_dbus_SRC ${kglobalaccel_component_xml} kglobalaccel_component_interface)

    target_sources(qhotkey PRIVATE ${kglobalaccel_dbus_SRC})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
endif()

include(GNUInstallDirs)

target_include_directories(qhotkey
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/QHotkey>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
