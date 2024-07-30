// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.workspace.dialogs as PWD
import org.kde.iconthemes as KIconThemes

PWD.SystemDialog {
    id: root

    property var dialog
    property string appID
    property url launcherURL: ""
    property bool edit: false

    readonly property Component displayComponent: Component {
        ColumnLayout {
            Kirigami.Icon {
                id: icon
                Layout.alignment: Qt.AlignHCenter
                implicitWidth: Kirigami.Units.iconSizes.enormous
                implicitHeight: implicitWidth
                source: dialog.icon
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                level: 3
                wrapMode: Text.Wrap
                text: dialog.name
                verticalAlignment: Qt.AlignTop
            }

            Kirigami.LinkButton {
                Layout.fillWidth: true
                visible: text.length > 0
                Layout.alignment: Qt.AlignHCenter
                elide: Text.ElideMiddle
                text: launcherURL
                onClicked: Qt.openUrlExternally(launcherURL)
            }
        }
    }

    readonly property Component editComponent: Component {
        ColumnLayout {
            QQC2.Button {
                Layout.alignment: Qt.AlignHCenter
                contentItem: Kirigami.Icon {
                    id: icon
                    implicitHeight: implicitWidth
                    implicitWidth: Kirigami.Units.iconSizes.enormous
                    source: dialog.icon

                    KIconThemes.IconDialog {
                        id: iconDialog
                        onIconNameChanged: dialog.icon = iconName
                    }

                    TapHandler {
                        onTapped: iconDialog.open()
                    }
                }
            }

            QQC2.Label {
                text: i18nc("@label name of a launcher/application", "Name")
                Layout.fillWidth: true
            }
            QQC2.TextField {
                verticalAlignment: Qt.AlignTop
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                onTextChanged: dialog.name = text
                Component.onCompleted: text = dialog.name
            }
        }
    }

    Loader {
        sourceComponent: root.edit ? editComponent : displayComponent
    }

    actions: [
        Kirigami.Action {
            text: i18nc("@action edit launcher name/icon", "Edit Info…")
            icon.name: "document-edit"
            onCheckedChanged: root.edit = checked
            checkable: true
        },
        Kirigami.Action {
            text: i18nc("@action accept dialog and create launcher", "Accept")
            icon.name: "dialog-ok"
            onTriggered: accept()
        },
        Kirigami.Action {
            text: i18nc("@action", "Cancel")
            icon.name: "dialog-cancel"
            onTriggered: reject()
        }
    ]
}
