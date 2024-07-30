/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

ColumnLayout {
    id: root

    property alias destinationPath: destination.text
    property var urls

    Label {
        text: i18n("Save directory:")
    }
    Button {
        id: destination
        Layout.fillWidth: true

        icon.name: "document-open-folder"
        onClicked: {
            dirSelector.visible = true
        }

        FolderDialog {
            id: dirSelector
            title: i18n("Select a directory where to save your pictures and videos")

            onAccepted: {
                destination.text = dirSelector.selectedFolder
            }
        }
    }
    Item {
        Layout.fillHeight: true
    }
}
