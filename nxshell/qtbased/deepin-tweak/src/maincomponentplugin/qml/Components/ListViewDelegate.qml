// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0
import "../Utils"

Rectangle {
    property bool isCurrentIndex
    property string _title: title
    property string _icon: icon
    signal clicked()
    id: item
    radius: Utils.listRadius
    color: Utils.transparentBackground
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            item.clicked()
            item.color = Utils.transparentBackground
        }
        onEntered: {
            if (isCurrentIndex) {
                return;
            }
            item.color = palette.base
        }
        onExited: {
            item.color = Utils.transparentBackground
        }
    }
    RowLayout {
        Layout.alignment: Qt.AlignVCenter
        anchors.fill: parent
        Rectangle {
            width: 10
            visible: isCurrentIndex
        }
        Image {
            source: _icon
            fillMode: Image.PreserveAspectFit
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.preferredWidth: 22
            Layout.preferredHeight: 22
            Layout.leftMargin: 5
        }
        Text {
            text: _title
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            font: DTK.fontManager.t8
        }
    }
}
