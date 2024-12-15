// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import Qt.labs.qmlmodels 1.2
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0

Rectangle {
    width: parent.width
    height: parent.height
    color: "transparent"
    clip: true
    property int processVal: 0
    ColumnLayout {
        width: parent.width
     //   Layout.fillHeight: true
        spacing: 10

        anchors.centerIn: parent
        Image {
            Layout.alignment: Qt.AlignHCenter
            visible: true
            source: oceanuiData.model().checkUpdateIcon
        }

        D.ProgressBar {
            Layout.alignment: Qt.AlignHCenter
            id: process
            from: 0
            to: 100
            value: processVal
            implicitHeight: 8
            implicitWidth: 160
            visible:oceanuiData.model().checkProcessRunning
        }

        D.Label {
            Layout.alignment: Qt.AlignHCenter
            width: implicitWidth
            text: oceanuiData.model().checkUpdateStateTips
            font.pixelSize: 12
        }

        D.Button {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: 200
            font.pixelSize: 14
            visible:!oceanuiData.model().checkProcessRunning
            text: (oceanuiData.model().checkUpdateState === 1 && !oceanuiData.model().checkProcessRunning) ? qsTr("Check Again") : qsTr("Check for Updates")
            onClicked: {
                oceanuiData.work().checkUpgrade();
            }
        }

        D.Label {
            visible: false
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Last checking time")
            font.pixelSize: 10
        }

        Timer {
            id: timer
            interval: 50
            repeat: true    // 设置为重复
            running: oceanuiData.model().checkProcessRunning
            onTriggered: {
                processVal++
                if (processVal > 100) {
                    oceanuiData.work().checkProcessStop();
                    processVal = 0
                }
            }
        }
    }
}


