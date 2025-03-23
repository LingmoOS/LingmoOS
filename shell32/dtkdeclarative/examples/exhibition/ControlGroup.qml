// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0
import ".."

 ControlGroup {
    title: "磁盘"
    ControlGroupItem {
        Label {
            visible: true
            text: "test0111111111111"
        }
        Label {
            visible: true
            text: "test13333333333"
        }
    }
    ControlGroupItem {
        Rectangle {
            width: 100
            height: 100
            color: "red"
            border.color: "black"
            border.width: 5
            radius: 10
        }
        Switch {
            checked: true
            Layout.alignment: Qt.AlignHCenter
        }
    }
    ControlGroupItem {
        Rectangle {
            width: 100
            height: 100
            color: "green"
            border.color: "black"
            border.width: 5
            radius: 10
        }
        Button {
            width: 100
            height: 100
            Layout.alignment: Qt.AlignHCenter
        }
    }
 }
