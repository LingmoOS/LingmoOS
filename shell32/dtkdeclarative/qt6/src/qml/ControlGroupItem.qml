// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQml.Models 2.11
import org.deepin.dtk 1.0 as D

RowLayout {
    id: root

    property int initY
    property bool isExpanded: true

    y: isExpanded ? initY : - parent.parent.titleHeight
    opacity: isExpanded ? 1.0 : 0.0
    visible: opacity === 0.0 ? false : true

    Layout.fillWidth: true
    Layout.fillHeight: true

    Behavior on y {
        enabled : D.DTK.hasAnimation
        NumberAnimation {
            duration: parent.parent.interval
            easing.type: Easing.Linear
        }
    }
    Behavior on opacity {
        enabled : D.DTK.hasAnimation
        NumberAnimation {
            duration: parent.parent.interval
            easing.type: Easing.Linear
        }
    }

    Component.onCompleted: {
        root.initY = root.y
    }
}
