// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.7
import org.deepin.dtk 1.0

Frame {
    id: frame
    property string displayName: qsTr("Automate Tasks")
    property string description: qsTr("Visual configuration automatically executes actions")
    property string version: "0.0.1"
    property string author: "myml"
    property string icon: "icon.png"
    Actions {
        id: actions
    }
    ColumnLayout {
        id: main
        property int delay: 0
        Layout.fillWidth: true
        Layout.fillHeight: true
        RowLayout {
            Text {
                text: qsTr("after")
            }
            SpinBox {
                id: control
                stepSize: 5
                from: 1
                to: 100000
                value: 30
                editable: true
                Layout.preferredWidth: 100
            }
            Text {
                text: qsTr("minutes")
                Layout.fillWidth: true
            }
        }
        RowLayout {
            Text {
                text: qsTr("perform")
            }
            ComboBox {
                id: action
                model: actions.getModel()
                textRole: "title"
                Layout.preferredWidth: 200
            }
            Text {
                text: qsTr("operation")
            }
        }
        Button {
            id: btn
            text: timer.running ? qsTr("Cancel") : qsTr("Start")
            Layout.fillWidth: true
            onClicked: {
                if (timer.running) {
                    timer.stop()
                } else {
                    main.delay = control.value*60-1
                    timer.start()
                }
            }
        }
        Text {
            visible: timer.running
            color: "red"
            font.pixelSize: 20
            wrapMode: Text.WordWrap
            Layout.preferredWidth: btn.width
            text: qsTr("%1 after %2:%3, please do not leave this UI.").
                arg(action.model.get(action.currentIndex).title).
                arg(Math.floor(main.delay/60)).
                arg(main.delay%60)
        }
        Timer {
            id: timer
            interval: 1000
            repeat: true
            onTriggered: {
                main.delay--
                if(main.delay <= 0){
                    timer.stop()
                    actions.execAction(action.currentIndex)
                }
            }
        }
    }
}