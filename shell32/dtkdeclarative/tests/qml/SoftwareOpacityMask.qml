// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0 as D

Rectangle {
    id: rootItem
    width: 200; height: 200

    Rectangle {
        id: rect1
        objectName: "rect1"
        layer.enabled: true
        width:50; height: 50
        color: "red"
        opacity: 0.5
    }

    Rectangle {
        id: rect2
        objectName: "rect2"
        x:50; y: 0
        layer.enabled: true
        width:50; height: 50
        color: "blue"
        opacity: 0.5
    }

    D.SoftwareOpacityMask {
        id: overlay
        x:100; y: 100
        width:50; height: 50
        source: rect1
        maskSource: rect2
        invert: true
    }
}
