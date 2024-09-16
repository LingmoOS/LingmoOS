# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

qt_add_qml_module(tray-quickpanel
    PLUGIN_TARGET tray-quickpanel
    URI "org.deepin.ds.dock.tray.quickpanel"
    VERSION "1.0"
    SOURCES
        quickpanelproxymodel.cpp
        quickpanelproxymodel.h
    QML_FILES
        QuickPanel.qml
        DragItem.qml
        PanelPluginPage.qml
        PanelTrayItem.qml
        PluginItem.qml
        QuickPanelModel.qml
        QuickPanelPage.qml
        SubPluginPage.qml
    OUTPUT_DIRECTORY
        "${PROJECT_BINARY_DIR}/plugins/org/deepin/ds/dock/tray/quickpanel"
)

target_link_libraries(tray-quickpanel PRIVATE
    Dtk${DTK_VERSION_MAJOR}::Core
)
