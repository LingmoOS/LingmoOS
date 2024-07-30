/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Window
import QtQuick.Controls as QQC2
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.networkmanagement as PlasmaNM

Window {
    id: dialog

    signal configurationDialogRequested()

    title: i18nc("@title:window", "Choose a Connection Type")

    height: 600
    minimumHeight: 400
    width: 500
    minimumWidth: 500

    PlasmaNM.CreatableConnectionsModel {
        id: connectionModel
    }

    Rectangle {
        id: background
        anchors.fill: parent
        focus: true
        color: Kirigami.Theme.backgroundColor
    }

    QQC2.ScrollView {
        id: scrollView
        anchors {
            bottom: buttonRow.top
            bottomMargin: Math.round(Kirigami.Units.gridUnit / 2)
            left: parent.left
            right: parent.right
            top: parent.top
        }

        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        ListView {
            id: view

            property int currentlySelectedIndex: -1
            property bool connectionShared
            property string connectionSpecificType
            property int connectionType
            property string connectionVpnType

            clip: true
            model: connectionModel
            currentIndex: -1
            boundsBehavior: Flickable.StopAtBounds
            section.property: "ConnectionTypeSection"
            section.delegate: Kirigami.ListSectionHeader {
                text: section
                width: ListView.view.width
            }
            Rectangle {
                id: background1
                z: -1
                anchors.fill: parent
                focus: true
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                color: Kirigami.Theme.backgroundColor
            }
            delegate: ListItem {
                height: connectionTypeBase.height
                width: view.width
                checked: view.currentlySelectedIndex === index

                onClicked: {
                    createButton.enabled = true
                    view.currentlySelectedIndex = index
                    view.connectionSpecificType = ConnectionSpecificType
                    view.connectionShared = ConnectionShared
                    view.connectionType = ConnectionType
                    view.connectionVpnType = ConnectionVpnType
                }

                onDoubleClicked: {
                    dialog.close()
                    kcm.onRequestCreateConnection(view.connectionType, view.connectionVpnType, view.connectionSpecificType, view.connectionShared)
                }

                Item {
                    id: connectionTypeBase

                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                        // Reset top margin from PlasmaComponents.ListItem
                        topMargin: -Math.round(Kirigami.Units.gridUnit / 3)
                    }
                    height: Math.max(Kirigami.Units.iconSizes.medium, connectionNameLabel.height + connectionDescriptionLabel.height) + Math.round(Kirigami.Units.gridUnit / 2)

                    Kirigami.Icon {
                        id: connectionIcon

                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        height: Kirigami.Units.iconSizes.medium; width: height
                        source: ConnectionIcon
                    }

                    Text {
                        id: connectionNameLabel

                        anchors {
                            bottom: ConnectionType === PlasmaNM.Enums.Vpn ? connectionIcon.verticalCenter : undefined
                            left: connectionIcon.right
                            leftMargin: Math.round(Kirigami.Units.gridUnit / 2)
                            right: parent.right
                            verticalCenter: ConnectionType === PlasmaNM.Enums.Vpn ? undefined : parent.verticalCenter
                        }
                        color: textColor
                        height: paintedHeight
                        elide: Text.ElideRight
                        text: ConnectionTypeName
                        textFormat: Text.PlainText
                    }

                    Text {
                        id: connectionDescriptionLabel

                        anchors {
                            left: connectionIcon.right
                            leftMargin: Math.round(Kirigami.Units.gridUnit / 2)
                            right: parent.right
                            top: connectionNameLabel.bottom
                        }
                        color: textColor
                        height: visible ? paintedHeight : 0
                        elide: Text.ElideRight
                        font.pointSize: Kirigami.Theme.smallFont.pointSize
                        opacity: 0.6
                        text: ConnectionDescription
                        visible: ConnectionType === PlasmaNM.Enums.Vpn
                    }
                }
            }
        }
    }

    Row {
        id: buttonRow
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: Math.round(Kirigami.Units.gridUnit / 2)
        }
        spacing: Kirigami.Units.mediumSpacing

        QQC2.Button {
            id: createButton
            icon.name: "list-add"
            enabled: false
            text: i18n("Create")

            onClicked: {
                dialog.close()
                kcm.onRequestCreateConnection(view.connectionType, view.connectionVpnType, view.connectionSpecificType, view.connectionShared)
            }
        }

        QQC2.Button {
            id: cancelButton
            icon.name: "dialog-cancel"
            text: i18n("Cancel")

            onClicked: {
                dialog.close()
            }
        }
    }

    QQC2.ToolButton {
        id: configureButton
        anchors {
            bottom: parent.bottom
            left: parent.left
            margins: Math.round(Kirigami.Units.gridUnit / 2)
        }
        icon.name: "configure"

        QQC2.ToolTip.text: i18n("Configuration")
        QQC2.ToolTip.visible: hovered

        onClicked: {
            dialog.configurationDialogRequested();
        }
    }
}
