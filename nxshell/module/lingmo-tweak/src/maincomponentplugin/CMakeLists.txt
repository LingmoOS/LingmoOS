# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

# SPDX-License-Identifier: CC0-1.0
set(MAINCOMPONENT_LIB_NAME ${CMAKE_PROJECT_NAME}-main)
set(MAINCOMPONENNT_SRC
    maincomponentplugin.h
    maincomponentplugin.cpp
    pluginlistmodel.h
    pluginlistmodel.cpp
    staff/launcher.h
    staff/launcher.cpp
    staff/file.h
    staff/file.cpp
    staff/staff.h
    staff/settings.h
    staff/settings.cpp
    staff/gsettings.h
    staff/gsettings.cpp
)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt5QuickCompiler REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
qtquick_compiler_add_resources(MAINCOMPONENT_RCC_SOURCES maincomponent.qrc)
include_directories(src/maincomponentplugin)
add_library(${MAINCOMPONENT_LIB_NAME} SHARED
    ${MAINCOMPONENNT_SRC}
    ${MAINCOMPONENT_RCC_SOURCES}
)
target_include_directories(${MAINCOMPONENT_LIB_NAME}
    PUBLIC
    ${DTKDECLARATIVE_INCLUDE_DIR}
    PkgConfig::QGSettings
)
target_link_libraries(${MAINCOMPONENT_LIB_NAME}
    Qt5::Quick
    Qt5::Widgets
    PkgConfig::QGSettings
    ${DtkDeclarative_LIBRARIES}
)
install(TARGETS ${MAINCOMPONENT_LIB_NAME} DESTINATION ${LIB_INSTALL_DIR})
