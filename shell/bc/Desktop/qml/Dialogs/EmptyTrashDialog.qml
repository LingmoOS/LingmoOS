/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import LingmoUI 1.0 as LingmoUI
import Lingmo.FileManager 1.0

LingmoUI.Window {
    id: control

    title: qsTr("File Manager")
    flags: Qt.Dialog | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    minimizeButtonVisible: false
    visible: true

    property int contentWidth: 300 + LingmoUI.Units.largeSpacing * 2
    property int contentHeight: _mainLayout.implicitHeight + control.header.height + LingmoUI.Units.largeSpacing

    width: contentWidth
    height: contentHeight

//    x: Screen.virtualX + (Screen.width - contentWidth) / 2
//    y: Screen.virtualY + (Screen.height - contentHeight) / 2

    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight

    headerBackground.color: LingmoUI.Theme.secondBackgroundColor

    DragHandler {
        target: null
        acceptedDevices: PointerDevice.GenericPointer
        grabPermissions: PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
        onActiveChanged: if (active) { control.helper.startSystemMove(control) }
    }

    Fm {
        id: fm
    }

    Rectangle {
        anchors.fill: parent
        color: LingmoUI.Theme.secondBackgroundColor
    }

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.topMargin: 0
        anchors.leftMargin: LingmoUI.Units.largeSpacing
        anchors.rightMargin: LingmoUI.Units.largeSpacing
        anchors.bottomMargin: LingmoUI.Units.largeSpacing
        spacing: LingmoUI.Units.largeSpacing

        Label {
            text: qsTr("Do you want to permanently delete all files from the Trash?")
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        RowLayout {
            spacing: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: control.close()
            }

            Button {
                text: qsTr("Empty Trash")
                focus: true
                Layout.fillWidth: true
                onClicked: {
                    fm.emptyTrash()
                    control.close()
                }
                flat: true
            }
        }
    }
}
