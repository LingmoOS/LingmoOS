// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0

Button {
    id: root
    property color color

    checkable: true
    background: Item {
        implicitWidth: 22
        implicitHeight: 22

        Rectangle {
            anchors {
                fill: parent
                margins: 3
            }

            radius: width / 2
            color: root.color
        }

        Rectangle {
            visible: root.checked
            anchors.fill: parent
            color: "transparent"
            radius: width / 2
            border {
                width: 1.5
                color: root.color
            }
        }
    }
}
