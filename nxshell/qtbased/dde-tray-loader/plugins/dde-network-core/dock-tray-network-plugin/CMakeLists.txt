# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "dock-tray-network-plugin")

project(${PLUGIN_NAME})
include(GNUInstallDirs)
# 启用 qt moc 的支持
set(CMAKE_AUTOMOC ON)
# 启用 qrc 资源文件的支持
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SRCS
    "networkplugin.h"
    "networkplugin.cpp"
    "quickpanelwidget.h"
    "quickpanelwidget.cpp"
    "dockcontentwidget.h"
    "../net-view/*.h"
    "../net-view/*.cpp"
    "widget/jumpsettingbutton.h"
    "widget/jumpsettingbutton.cpp"
    "widget/commoniconbutton.h"
    "widget/commoniconbutton.cpp"
)

find_package(Qt5 COMPONENTS Core Widgets DBus Network LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
endif()

file(GLOB TS_FILES "translations/*.ts")
qt5_add_translation(QM_FILES ${TS_FILES})
add_custom_target(language ALL DEPENDS ${QM_FILES})

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} ../net-view/window/qrc/network.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../dde-dock)

get_target_property(KF5_QT_INCLUDE_DIRS KF5::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

set(TRY_KF5_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

if (NOT KF5_QT_LIBRARIES)
    find_library(KF5_QT_LIBRARIES libKF5NetworkManagerQt.so)
endif()

target_include_directories(${PLUGIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${KF5_QT_INCLUDE_DIRS}
    "../../../interfaces"
    "../net-view/operation"
    "../net-view/window"
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${KF5_QT_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    dde-tray-network-core
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/system-trays)
# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PLUGIN_NAME}/translations)
# 安装显示在控制中心-个性化-任务栏-插件区域的图标
install(FILES "resources/network.svg" DESTINATION share/dde-dock/icons/dcc-setting)
