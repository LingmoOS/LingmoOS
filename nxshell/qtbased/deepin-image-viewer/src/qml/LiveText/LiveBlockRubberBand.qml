// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import org.deepin.image.viewer 1.0 as IV

Item {
    id: root

    function setActiveColor(color) {
        rect.color = color;
    }

    visible: false

    Component.onCompleted: {
        rect.color = IV.CursorTool.activeColor();
        IV.CursorTool.activeColorChanged.connect(setActiveColor);
    }

    Rectangle {
        id: rect

        anchors.fill: parent
        opacity: 0.15
    }
}
