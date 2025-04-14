/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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

    flags: Qt.Dialog | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    minimizeButtonVisible: false
    visible: true

    width: contentWidth
    height: contentHeight

    minimumWidth: contentWidth
    minimumHeight: contentHeight
    maximumWidth: contentWidth
    maximumHeight: contentHeight

    property var contentWidth: _mainLayout.implicitWidth + LingmoUI.Units.largeSpacing * 2
    property var contentHeight: _mainLayout.implicitHeight + control.header.height + LingmoUI.Units.largeSpacing * 2

    background.color: LingmoUI.Theme.secondBackgroundColor

    ColumnLayout {
        id: _mainLayout
        anchors.fill: parent
        anchors.leftMargin: LingmoUI.Units.largeSpacing
        anchors.rightMargin: LingmoUI.Units.largeSpacing
        anchors.bottomMargin: LingmoUI.Units.smallSpacing
        spacing: LingmoUI.Units.largeSpacing

        Label {
            text: fileCount === 1 ? qsTr("Are you sure you want to permanently delete \"%1\"?")
                                   .arg(urls[0].toString().split('/').pop())
                                 : qsTr("Are you sure you want to permanently delete %1 files?")
                                   .arg(fileCount)
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        Label {
            text: qsTr("Once deleted, these files cannot be recovered.")
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            color: LingmoUI.Theme.disabledTextColor
        }

        RowLayout {
            spacing: LingmoUI.Units.largeSpacing

            Button {
                text: qsTr("Cancel")
                Layout.fillWidth: true
                onClicked: control.close()
            }

            Button {
                text: qsTr("Delete")
                focus: true
                Layout.fillWidth: true
                onClicked: {
                    control.close()
                    model.deleteSelected()
                }
                flat: true
            }
        }
    }
} 