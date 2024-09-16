# SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-only

PROJECT(dde-dconfig-editor)
CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

set(REQUIRED_QT_VERSION 5.11.3)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Widgets DBus LinguistTools)
find_package(Dtk${DTK_VERSION_MAJOR}Core REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR}Gui REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR}Widget REQUIRED)

# generate moc_predefs.h
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

set(DCONFIG_DBUS_XML_CONFIGMANAGER ../dde-dconfig-daemon/services/org.desktopspec.ConfigManager.xml)
set_source_files_properties(${DCONFIG_DBUS_XML_CONFIGMANAGER} PROPERTIES CLASSNAME DSGConfig NO_NAMESPACE NO_NAMESPACE)
if(EnableDtk5)
    qt5_add_dbus_interface(DCONFIG_DBUS_XML ${DCONFIG_DBUS_XML_CONFIGMANAGER} manager_interface)
endif()
if(EnableDtk6)
    qt_add_dbus_interface(DCONFIG_DBUS_XML ${DCONFIG_DBUS_XML_CONFIGMANAGER} manager_interface)
endif()

set(DCONFIG_DBUS_XML_MANAGER ../dde-dconfig-daemon/services/org.desktopspec.ConfigManager.Manager.xml)
set_source_files_properties(${DCONFIG_DBUS_XML_MANAGER} PROPERTIES CLASSNAME DSGConfigManager NO_NAMESPACE NO_NAMESPACE)
if(EnableDtk5)
    qt5_add_dbus_interface(DCONFIG_DBUS_XML ${DCONFIG_DBUS_XML_MANAGER} configmanager_interface)
endif()
if(EnableDtk6)
    qt_add_dbus_interface(DCONFIG_DBUS_XML ${DCONFIG_DBUS_XML_MANAGER} configmanager_interface)
endif()

set(HEADERS
    mainwindow.h
    ../common/valuehandler.h
    ../common/helper.hpp
    iteminfo.h
    exportdialog.h
    oemdialog.h
)
set(SOURCES
    main.cpp
    mainwindow.cpp
    ../common/valuehandler.cpp
    iteminfo.cpp
    exportdialog.cpp
    oemdialog.cpp
)

set(TRANSLATIONS translates/dde-dconfig-editor_zh_CN.ts)
if(EnableDtk6)
    qt6_add_translation(QM_FILES ${TRANSLATIONS})
endif()
if (EnableDtk5)
    qt5_add_translation(QM_FILES ${TRANSLATIONS})
endif()

set(QRC dde-dconfig-editor.qrc)

ADD_EXECUTABLE(dde-dconfig-editor ${HEADERS} ${SOURCES} ${QRC} ${DCONFIG_DBUS_XML} ${QM_FILES})

set(COMMON_LIBS
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkWidget_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${COMMON_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ${DtkWidget_INCLUDE_DIRS}
    ../common
)

install(TARGETS dde-dconfig-editor DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/dde-dconfig-editor/translations/)
