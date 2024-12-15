# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widget)

file(GLOB SRCS "*.h" "*.cpp" "../../interfaces/*.h" "utils/*.h" "utils/*.cpp")

file(GLOB TS_FILES "translations/*.ts")
qt_add_translation(QM_FILES ${TS_FILES})
add_custom_target(trayloader_translations ALL DEPENDS ${QM_FILES})

add_executable(trayplugin-loader
    main.cpp
    dockdbusproxy.cpp
    dockdbusproxy.h
    widgetplugin.h
    widgetplugin.cpp
    pluginitem.h
    pluginitem.cpp
    quickpluginitem.h
    quickpluginitem.cpp
    utils/setproctitle.h
    utils/setproctitle.cpp
    pluginmanager.h
    pluginmanager.cpp
    loader.qrc
)

target_include_directories(trayplugin-loader PUBLIC
    "utils/"
    "../../interfaces/"
    "../tray-wayland-integration/"
    "${CMAKE_BINARY_DIR}/"
)

target_link_libraries(trayplugin-loader PRIVATE
    dockpluginmanager-interface
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Dtk${DTK_VERSION_MAJOR}::Widget
    Dtk${DTK_VERSION_MAJOR}::Gui
)

install(TARGETS trayplugin-loader DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

#dconfig
dtk_add_config_meta_files(APPID org.lingmo.ocean.dock FILES ./configs/org.lingmo.ocean.dock.plugin.common.json) # compat
dtk_add_config_meta_files(APPID org.lingmo.ocean.tray-loader FILES ./configs/org.lingmo.ocean.dock.plugin.common.json)

# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/trayplugin-loader/translations)
