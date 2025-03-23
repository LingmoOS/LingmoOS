// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0 as D

Rectangle {
    id: rootItem
    width: 100; height:100
    color: "blue"
    Image {
        id: img
        objectName: "image"
        width:50; height: 50
        source: "qrc:/icon/deepin.png"
    }

    D.SoftwareColorOverlay {
        id: overlay
        width: 50; height: 50
        source: img
        color: "red"
    }
}
