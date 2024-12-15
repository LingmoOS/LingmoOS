# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation prefix")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} COMPONENTS Core Widgets DBus Svg LinguistTools REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget Core Tools)

set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")
file(GLOB COMMON_FILE "${COMMON_DIR}/*.cpp")

#--------------------------dock-plugin---------------------------
set(Dock_Plugin_Name dock-wirelesscasting-plugin)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dock-wirelesscasting-plugin/src")
set(TYPES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dock-wirelesscasting-plugin/dbus/types")

# 使用 file() 命令和 GLOB 参数获取目录下的所有文件
file(GLOB DOCK_PLUGIN_SOURCE_FILES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${SOURCE_DIR}/widget/*.h"
    "${SOURCE_DIR}/widget/*.cpp"
    "${SOURCE_DIR}/../dbus/*.cpp"
    "${TYPES_SOURCE_DIR}/*.h"
    "${TYPES_SOURCE_DIR}/*.cpp"
)

dtk_add_dbus_interface(
    DBUS_INTERFACES
    ${CMAKE_CURRENT_SOURCE_DIR}/dock-wirelesscasting-plugin/dbus/xml/org.lingmo.ocean.Display1.xml
    display1interface
)

# 将获取到的源文件添加到目标可执行文件中
add_library(${Dock_Plugin_Name} SHARED
    ${DBUS_INTERFACES}
    ${DOCK_PLUGIN_SOURCE_FILES}
    ${COMMON_FILE}
    dock-wirelesscasting-plugin/res/resources.qrc
    dock-wirelesscasting-plugin/translations/translations.qrc
)

set_target_properties(${Dock_Plugin_Name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../ocean-dock)

target_include_directories(${Dock_Plugin_Name} PUBLIC
    ./dock-wirelesscasting-plugin/dbus
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    ${OBJECT_BINARY_DIR}
    ${COMMON_DIR}
    "../../../interfaces"
    )
target_link_libraries(dock-wirelesscasting-plugin
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${Dock_Plugin_Name} LIBRARY DESTINATION lib/ocean-dock/plugins)
# 翻译文件
file(GLOB TS_FILES_DOCK "dock-wirelesscasting-plugin/translations/*.ts")
qt_add_translation(QM_FILES_DOCK ${TS_FILES_DOCK})
add_custom_target(dock_translations ALL DEPENDS ${QM_FILES_DOCK})
install(FILES ${QM_FILES_DOCK} DESTINATION share/dock-wirelesscasting-plugin/translations)
# 安装显示在控制中心-个性化-任务栏-插件区域的图标
install(FILES "dock-wirelesscasting-plugin/res/wireless-casting.svg" DESTINATION share/ocean-dock/icons/oceanui-setting)
