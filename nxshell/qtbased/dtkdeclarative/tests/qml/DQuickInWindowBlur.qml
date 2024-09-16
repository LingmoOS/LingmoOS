// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D

Rectangle {
    width: 200; height: 200
    color: "blue"
    Image {
        width: 100; height: 100
        source: "qrc:/icon/deepin.png"
        D.InWindowBlur {
            width: 80; height: 80
            radius: 10
            offscreen: false
        }
    }
}
