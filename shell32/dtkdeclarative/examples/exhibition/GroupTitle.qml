// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick.Controls 2.4
import QtQuick 2.0
import QtQuick.Layouts 1.0

Item {
    property alias text: label.text
    signal viewSource

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight + 20

    RowLayout {
        id: layout
        anchors.fill: parent
        spacing: 10

        Label {
            id: label
            Layout.alignment: Qt.AlignVCenter
        }
        Rectangle {
            id: line
            color: Qt.rgba(0, 0, 0, 0.08)
            height: 1
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
        }
        Button {
            id: button
            text: "查看源码"
            Layout.alignment: Qt.AlignVCenter
            onClicked: viewSource()
        }
    }
}
