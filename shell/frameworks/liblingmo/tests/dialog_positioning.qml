/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore


/*This test is for checking LingmoDialog visualParent and related function work
*To test move the window towards various edges and press the button.
The Red rectangle should always be on screen and on the right screen

*/

LingmoCore.Dialog {

    type: windowIsDockControl.checked ? LingmoCore.Dialog.Dock : LingmoCore.Dialog.Normal
    visible: true

    Rectangle {
        color: "#ffffff"
        width: 300
        height: 300

        Rectangle {
            id: innerRect
            color: "#ddffdd"
            width: 200
            height: layout.height
            anchors.centerIn: parent

            ColumnLayout {
                id: layout
                anchors.margins: 5
                anchors.top: parent.top
                anchors.left:parent.left
                anchors.right:parent.right

                Controls.Label {
                    Layout.fillWidth: true
                    text: "alt + left click and move the window to various edges to test popup position"
                    wrapMode: Text.WordWrap
                }

                Controls.ComboBox {
                    id: alignmentControl
                    //order must match Location in lingmo.h
                    model: ["Left", "Right", "Top", "Bottom"]
                    currentIndex: 0
                }

                Controls.CheckBox {
                    id: windowIsDockControl
                    text: "Window is a dock"
                }

                Controls.Button {
                    text: "Show Popup"
                    onClicked: {
                        dialog.visible = !dialog.visible
                        console.log(alignmentControl.currentIndex);
                    }
                }
            }

            LingmoCore.Dialog
            {
                id: dialog
                visualParent: innerRect
                location: {
                    switch (alignmentControl.currentIndex) {
                        case 0:
                            return LingmoCore.Types.LeftEdge
                        case 1:
                            return LingmoCore.Types.RightEdge
                        case 2:
                            return LingmoCore.Types.TopEdge
                        default:
                            return LingmoCore.Types.BottomEdge
                    }
                }


                Rectangle {
                    color: "#FF0000"
                    width: 150
                    height: 150
                }

                Component.onCompleted: {
                    console.log(alignmentControl.currentIndex);
                    console.log(dialog.location);

                }
            }
        }
    }
}
