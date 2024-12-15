# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "notification")

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS DBus)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget Gui)

add_library(${PLUGIN_NAME} SHARED
    notification.h
    notification.cpp
    notificationplugin.h
    notificationplugin.cpp
    notification.qrc
    ../widgets/tipswidget.h
    ../widgets/tipswidget.cpp
)
target_compile_definitions(${PLUGIN_NAME} PRIVATE QT_PLUGIN)
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../)

target_include_directories(${PLUGIN_NAME} PRIVATE
   "../../../interfaces"
   "../widgets"
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::DBus
    Dtk${DTK_VERSION_MAJOR}::Gui
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/ocean-dock/plugins)
install(FILES "icons/oceanui-notification.dci" DESTINATION share/ocean-dock/icons/oceanui-setting)
