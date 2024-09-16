// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick

import org.deepin.ds.dock.tray.quickpanel 1.0

QuickPanelProxyModel {
    id: root

    signal requestShowSubPlugin(var pluginId, var subSurface)
}
