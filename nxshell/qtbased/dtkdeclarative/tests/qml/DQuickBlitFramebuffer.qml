// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0 as D

Rectangle {
    width: 200; height: 200
    color: "red"
    D.BlitFramebuffer {
        anchors.centerIn: parent
        width: 100;  height: 100
        Rectangle {
            width: 100;  height: 100
            color: "blue"
        }
    }
}
