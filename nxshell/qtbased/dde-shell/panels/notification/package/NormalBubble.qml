// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

D.Control {
    id: control
    property var bubble

    contentItem: BubbleContent {
        bubble: control.bubble
    }

    z: bubble.level <= 1 ? 0 : 1 - bubble.level

    background: Rectangle {
        width: 340
        radius: 18
        opacity: {
            if (bubble.level === 1)
                return 0.8
            return 1
        }
    }
}
