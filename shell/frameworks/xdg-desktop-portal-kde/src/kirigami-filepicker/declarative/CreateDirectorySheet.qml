// SPDX-FileCopyrightText: 2020 Jonah Brüchert <jbb@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamifilepicker

Kirigami.OverlaySheet {
    id: sheet

    property string parentPath

    header: Kirigami.Heading {
        text: i18n("Create New Folder")
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true

            wrapMode: Text.WordWrap
            text: i18n("Create new folder in %1", sheet.parentPath.replace("file://", ""))
        }

        QQC2.TextField {
            id: nameField
            Layout.fillWidth: true

            placeholderText: i18n("Folder name")
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.fillWidth: true

            QQC2.Button {
                Layout.alignment: Qt.AlignLeft
                Layout.fillWidth: true

                text: i18n("OK")

                onClicked: {
                    DirModelUtils.mkdir(parentPath + "/" + nameField.text)
                    sheet.close()
                }
            }
            QQC2.Button {
                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true

                text: i18n("Cancel")

                onClicked: {
                    nameField.clear()
                    sheet.close()
                }
            }
        }
    }
}
