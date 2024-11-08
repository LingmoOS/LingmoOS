/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ColumnLayout
{
    height: LingmoUI.Units.gridUnit * 20
    Controls.Label {
        id: label
        text: "Press the button and make sure the popup is on the correct place"
        wrapMode: Text.WordWrap
    }
    LingmoComponents.Button {
        id: settingsButton
        iconSource: "configure"
        text: "Press Me"
        Layout.alignment: Qt.AlignHCenter
        rotation: 90

        onClicked: {
            contextMenu.visible = !contextMenu.visible;
        }
    }

    LingmoCore.Dialog {
        id: contextMenu
        visualParent: settingsButton

        location: LingmoCore.Types.BottomEdge
        type: LingmoCore.Dialog.PopupMenu
        flags: Qt.Popup | Qt.FramelessWindowHint | Qt.WindowDoesNotAcceptFocus

        mainItem: ColumnLayout {
            id: menuColumn
            Layout.minimumWidth: menuColumn.implicitWidth
            Layout.minimumHeight: menuColumn.implicitHeight
            spacing: LingmoUI.Units.smallSpacing

            LingmoUI.Heading {
                level: 3
                text: "Panel Alignment"
            }

            LingmoComponents.ButtonColumn {
                spacing: 0
                Layout.fillWidth: true
                LingmoComponents.ToolButton {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "Left"
                    checkable: true
                    flat: false
                }
                LingmoComponents.ToolButton {
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    text: "Center"
                    checkable: true
                    flat: false
                }
            }
        }
    }
}
