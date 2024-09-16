// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Window 2.15
import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

PanelPopupWindow {
    id: root

    flags: (Qt.platform.pluginName === "xcb" ? Qt.Tool : Qt.ToolTip) | Qt.WindowStaysOnTopHint
    D.DWindow.windowRadius: 8
    D.DWindow.shadowRadius: 8
}
