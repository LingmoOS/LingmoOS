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

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} COMPONENTS Core Widgets DBus Network Svg LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget)
find_package(KF6NetworkManagerQt REQUIRED)

#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
#endif()

file(GLOB TS_FILES "translations/*.ts")
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(language ALL DEPENDS ${QM_FILES})

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} ../net-view/window/qrc/network.qrc)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../ocean-dock)

get_target_property(KF6_QT_INCLUDE_DIRS KF6::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

set(TRY_KF6_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

if (NOT KF6_QT_LIBRARIES)
    find_library(KF6_QT_LIBRARIES libKF6NetworkManagerQt.so)
endif()

target_include_directories(${PLUGIN_NAME} PUBLIC
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Dbus
    Qt${QT_VERSION_MAJOR}::Network
    KF6::NetworkManagerQt
    "../../../interfaces"
    "../net-view/operation"
    "../net-view/window"
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Dtk${DTK_VERSION_MAJOR}::Widget
    KF6::NetworkManagerQt
    ocean-tray-network-core
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins/system-trays)
# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PLUGIN_NAME}/translations)
# 安装显示在控制中心-个性化-任务栏-插件区域的图标
install(FILES "resources/oceanui-network.dci" DESTINATION share/ocean-dock/icons/oceanui-setting)
