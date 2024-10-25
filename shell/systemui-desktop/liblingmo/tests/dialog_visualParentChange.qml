/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore

ColumnLayout {
    Controls.Label {
        Layout.maximumWidth: mainLayout.width
        wrapMode: Text.WordWrap
        text: "Click on each coloured box to make a dialog popup. It should popup in the correct position. The popup should also move from one rectangle to the other on hovering"
    }

    RowLayout {
        id: mainLayout
        Rectangle {
            width: 300
            height: 100
            color: "red"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "blue"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "green"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "yellow"

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    dialog.visualParent = parent;
                    dialog.visible = !dialog.visible;
                }
                onEntered: {
                    dialog.visualParent = parent;
                }
            }
        }

        LingmoCore.Dialog {
            id: dialog
            location: LingmoCore.Types.BottomEdge
            visible: false

            Rectangle {
                color: "black"
                width: 150
                height: 150
            }
        }
    }
}
