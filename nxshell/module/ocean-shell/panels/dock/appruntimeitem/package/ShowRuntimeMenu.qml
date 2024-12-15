// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1

import WindowModule 1.0
import XcbModule 1.0

ApplicationWindow {
    visible: true
    width: 800
    height: 400
    title: "AppMonitor"
    Component.onCompleted: {
        XcbGetInstance.getAllOpenedWiondws();
        XcbGetInstance.startGetinfo();
    }
    Column {
        anchors.fill: parent

        Row {
            spacing: 10
            width: parent.width
            height: 40

            Text { text: "AppName"; width: parent.width / 2 }
            Text { text: "WindowsId"; width: parent.width / 4 }
            Text { text: "RunTime"; width: parent.width / 4 }
        }

        ListView {
            id: listView
            anchors.fill: parent
            anchors.topMargin: 30
            model: WindowManagerInstance

            delegate: Item {
                width: listView.width
                height: 30

                Row {
                    width: parent.width
                    height: parent.height
                    spacing: 10

                    Text {
                        text: model.name
                        width: listView.width / 2
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignLeft
                    }
                    Text {
                        text: model.id
                        width: listView.width / 4
                    }
                    Text {
                        id: elapsedTimeText
                        text: "00h:00m:00s"
                        width: listView.width / 4
                    }

                    Timer {
                        id: timer
                        interval: 10000
                        running: true
                        repeat: true
                        onTriggered: {
                            var now = new Date();
                            var elapsedTime = now - model.startTime;

                            var hours = Math.floor(elapsedTime / (1000 * 60 * 60));
                            elapsedTime %= (1000 * 60 * 60);

                            var minutes = Math.floor(elapsedTime / (1000 * 60));
                            elapsedTime %= (1000 * 60);

                            var seconds = Math.floor(elapsedTime / 1000);

                            var displayText = "";
                            displayText += (hours < 10 ? "0" + hours : hours) + "h:";
                            displayText += (minutes < 10 ? "0" + minutes : minutes) + "m:";
                            displayText += (seconds < 10 ? "0" + seconds : seconds) + "s";

                            elapsedTimeText.text = displayText;
                        }
                    }
                }
            }
        }
    }
}

