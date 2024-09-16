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
find_package(Qt5 COMPONENTS Core Widgets DBus Svg LinguistTools REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkCore REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")
file(GLOB COMMON_FILE "${COMMON_DIR}/*.cpp")

#--------------------------dock-plugin---------------------------
message("Complie dock plugin")
set(Dock_Plugin_Name dock-wirelesscasting-plugin)

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dock-wirelesscasting-plugin/src")

# 使用 file() 命令和 GLOB 参数获取目录下的所有文件
file(GLOB DOCK_PLUGIN_SOURCE_FILES
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
    "${SOURCE_DIR}/widget/*.h"
    "${SOURCE_DIR}/widget/*.cpp"
)

# 将获取到的源文件添加到目标可执行文件中
add_library(${Dock_Plugin_Name} SHARED
${DOCK_PLUGIN_SOURCE_FILES}
${COMMON_FILE}
dock-wirelesscasting-plugin/res/resources.qrc
dock-wirelesscasting-plugin/translations/translations.qrc
)

set_target_properties(${Dock_Plugin_Name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../../dde-dock)

target_include_directories(${Dock_Plugin_Name} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    ${OBJECT_BINARY_DIR}
    ${DDE_DOCK_INCLUDE_DIR}
    ${COMMON_DIR}
    ${DFrameworkDBus_INCLUDE_DIRS}
    "../../../interfaces"
    )
target_link_libraries(dock-wirelesscasting-plugin
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${Dock_Plugin_Name} LIBRARY DESTINATION lib/dde-dock/plugins)
# 翻译文件
file(GLOB TS_FILES_DOCK "dock-wirelesscasting-plugin/translations/*.ts")
qt5_add_translation(QM_FILES_DOCK ${TS_FILES_DOCK})
add_custom_target(dock_translations ALL DEPENDS ${QM_FILES_DOCK})
install(FILES ${QM_FILES_DOCK} DESTINATION share/dock-wirelesscasting-plugin/translations)
# 安装显示在控制中心-个性化-任务栏-插件区域的图标
install(FILES "dock-wirelesscasting-plugin/res/wireless-casting.svg" DESTINATION share/dde-dock/icons/dcc-setting)
