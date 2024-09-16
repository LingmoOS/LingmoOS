// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.deepin.ds 1.0
import org.deepin.ds.dock.tray.quickpanel 1.0

QuickPanel {
    id: root
    property bool isHorizontal: false
    useColumnLayout: !isHorizontal
    trayItemPluginId: Applet.rootObject.quickpanelTrayItemPluginId
    trayItemMargins: itemPadding
}
