// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.lingmo.ds.notificationcenter

Loader {
    id: root
    property int radius: 12
    property color color: palette.base
    active: NotifyAccessor.debugging
    sourceComponent: Rectangle {
        border.color: "cyan"
        color: root.color
        radius: root.radius
    }
}
