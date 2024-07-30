/*
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Window 2.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.lingmoui as LingmoUI

Window {
    id: window

    function open(windowName, windowIcon) {
        window.title = windowName;
        window.icon = windowIcon;
        window.state = "open";
    }
    property alias state: background.state
    property alias icon: icon.source
    width: Screen.width
    height: Screen.height
    color: "transparent"
    onVisibleChanged: {
        if (!visible) {
            background.state = "closed";
        }
    }
    onActiveChanged: {
        if (!active) {
            background.state = "closed";
        }
    }

    Item {
        id: background
        anchors.fill: parent
        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary
        width: window.width
        height: window.height
        state: "closed"

        Rectangle {
            anchors.fill: parent
            color: LingmoUI.Theme.backgroundColor

            ColumnLayout {
                anchors.centerIn: parent
                LingmoUI.Icon {
                    id: icon
                    LingmoUI.Theme.inherit: false
                    LingmoUI.Theme.colorSet: LingmoUI.Theme.Complementary
                    Layout.preferredWidth: LingmoUI.Units.iconSizes.enormous
                    Layout.preferredHeight: Layout.preferredWidth
                    Layout.alignment: Qt.AlignCenter
                }
                LingmoExtras.Heading {
                    text: window.title
                    Layout.alignment: Qt.AlignCenter
                }
            }
        }

        states: [
            State {
                name: "closed"
                PropertyChanges {
                    target: background
                    scale: 0
                }
                PropertyChanges {
                    target: window
                    visible: false
                }
            },
            State {
                name: "open"
                PropertyChanges {
                    target: background
                    scale: 1
                }
                PropertyChanges {
                    target: window
                    visible: true
                }
            }
        ]

        transitions: [
            Transition {
                from: "closed"
                SequentialAnimation {
                    ScriptAction {
                        script: window.visible = true;
                    }
                    PropertyAnimation {
                        target: background
                        duration: units.longDuration
                        easing.type: Easing.InOutQuad
                        properties: "scale"
                    }
                }
            }
        ]
    }
}
