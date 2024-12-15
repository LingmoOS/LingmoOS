# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.12)


set(PLUGIN_NAME "oceanui-insider-plugin")

find_package(Qt5 COMPONENTS Core Widgets Concurrent LinguistTools REQUIRED)
find_package(Dtk COMPONENTS Widget REQUIRED)
find_package(QCoro5 COMPONENTS Core REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(PackageKitQt5 REQUIRED)

set(SOURCE_FILES
    insidermodule.cpp insidermodule.h
    insiderplugin.cpp insiderplugin.h
    pkutils.cpp pkutils.h
)

set(TRANSLATION_FILES
    ../translations/oceanui-insider-plugin.ts
    ../translations/oceanui-insider-plugin_zh_CN.ts
)

qt_create_translation(TRANSLATED_FILES ${SOURCE_FILES} ${TRANSLATION_FILES})

add_library(${PLUGIN_NAME} SHARED
    ${SOURCE_FILES}
    ${TRANSLATED_FILES}
)
set_property(TARGET ${PLUGIN_NAME} PROPERTY CXX_STANDARD 20)

target_include_directories(${PLUGIN_NAME}
PUBLIC
    ${DdeControlCenter_INCLUDE_DIR}
)

target_link_libraries(${PLUGIN_NAME}
PRIVATE
    Qt::Concurrent
    QCoro::Core
    Dtk::Widget
    ${DdeControlCenter_LIBRARIES}
    PK::packagekitqt5
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/ocean-control-center/modules)
install(FILES ${TRANSLATED_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ocean-control-center/translations)
