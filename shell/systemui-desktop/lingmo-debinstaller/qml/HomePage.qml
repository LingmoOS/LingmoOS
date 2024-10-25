/*
 * Copyright (C) 2021 Lingmo Technology Co., Ltd.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
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
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.2
import LingmoUI 1.0 as LingmoUI
import Lingmo.DebInstaller 1.0

Item {
    FileDialog {
        id: fileDialog
        folder: shortcuts.home
        nameFilters: ["Deb files (*.deb)", "All files (*)"]
        onAccepted: {
            Installer.fileName = fileDialog.fileUrl.toString().replace("file://", "")
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: LingmoUI.Units.largeSpacing * 2

        Item {
            Layout.fillHeight: true
        }

        Image {
            width: 96
            height: width
            sourceSize: Qt.size(width, height)
            source: "image://icontheme/application-x-deb"
            Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
        }

        Item {
            height: LingmoUI.Units.largeSpacing
        }

        Label {
            text: qsTr("You can drag the deb package here")
            Layout.alignment: Qt.AlignHCenter
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Select file")
            color: LingmoUI.Theme.highlightColor
            focus: true

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: fileDialog.open()
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
