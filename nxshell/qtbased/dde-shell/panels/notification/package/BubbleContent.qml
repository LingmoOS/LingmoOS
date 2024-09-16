// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

D.Control {
    property var bubble

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: {
            if (!bubble.hasDefaultAction)
                return

            console.log("default action", bubble.index)
            Applet.defaultActionInvoke(bubble.index)
        }
        property bool longPressed
        onPressAndHold: {
            longPressed = true
        }
        onPositionChanged: {
            if (longPressed) {
                longPressed = false
                console.log("delay process", bubble.index)
                Applet.delayProcess(bubble.index)
            }
        }
    }
    contentItem: RowLayout {
        D.QtIcon {
            Layout.leftMargin: 10
            sourceSize: Qt.size(40, 40)
            name: bubble.iconName
        }

        ColumnLayout {
            spacing: 0
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 40
            Text {
                visible: bubble.title !== ""
                Layout.alignment: Qt.AlignLeft
                elide: Text.ElideRight
                text: bubble.title
                Layout.fillWidth: true
                maximumLineCount: 1
                font: D.DTK.fontManager.t6
            }

            Text {
                visible: bubble.text !== ""
                Layout.alignment: Qt.AlignLeft
                elide: Text.ElideRight
                text: bubble.text
                Layout.fillWidth: true
                maximumLineCount: 2
                font: D.DTK.fontManager.t7
                textFormat: Text.PlainText
                wrapMode: Text.WrapAnywhere
            }
        }

        Loader {
            Layout.bottomMargin: 10
            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            active: bubble.hasDisplayAction
            sourceComponent: BubbleAction {
                bubble: control.bubble
                onActionInvoked: function(actionId) {
                    console.log("action", actionId, bubble.index)
                    Applet.actionInvoke(bubble.index, actionId)
                }
            }
        }
        
    }
    
    Button {
         id: closeBtn
         visible: control.hovered
         width: 18
         height: 18
         icon.name: "window-close"
         icon.width: 10
         icon.height: 10
         anchors.right: parent.right
         anchors.top: parent.top
         anchors.topMargin: -7
         anchors.rightMargin: -7
         z: control.z + 1
         onClicked: Applet.delayProcess(bubble.index)
    }
}
