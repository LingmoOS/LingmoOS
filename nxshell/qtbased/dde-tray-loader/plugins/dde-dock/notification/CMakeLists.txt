# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

set(PLUGIN_NAME "notification")
find_package(Qt5 REQUIRED COMPONENTS DBus)
find_package(Dtk REQUIRED COMPONENTS Widget Gui)

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
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ../dde-dock)

target_include_directories(${PLUGIN_NAME} PRIVATE
   "../../../interfaces"
   "../widgets"
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dtk::Widget
    Qt5::DBus
    # Dtk6::Widget
    Dtk::Gui
)

install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
install(FILES "icons/notification.svg" DESTINATION share/dde-dock/icons/dcc-setting)
