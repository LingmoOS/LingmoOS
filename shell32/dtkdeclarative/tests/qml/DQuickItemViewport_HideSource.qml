// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D

Rectangle {
    width: 200; height: 200
    color: "blue"
    Image {
        id: img
        width: 100; height: 100
        source: "qrc:/icon/deepin.png"
    }
    D.ItemViewport {
        width: 100; height: 100
        x: 100; y: 100
        radius: 10
        fixed: true
        hideSource: true
        sourceItem: img
    }
}
