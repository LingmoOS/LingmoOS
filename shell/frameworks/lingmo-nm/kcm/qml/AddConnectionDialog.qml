/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import QtQuick.Window
import QtQuick.Controls as QQC2
import org.kde.lingmoui 2 as LingmoUI
import org.kde.lingmo.networkmanagement as LingmoNM

Window {
    id: dialog

    signal configurationDialogRequested()

    title: i18nc("@title:window", "Choose a Connection Type")

    height: 600
    minimumHeight: 400
    width: 500
    minimumWidth: 500

    LingmoNM.CreatableConnectionsModel {
        id: connectionModel
    }

    Rectangle {
        id: background
        anchors.fill: parent
        focus: true
        color: LingmoUI.Theme.backgroundColor
    }

    QQC2.ScrollView {
        id: scrollView
        anchors {
            bottom: buttonRow.top
            bottomMargin: Math.round(LingmoUI.Units.gridUnit / 2)
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
            section.delegate: LingmoUI.ListSectionHeader {
                text: section
                width: ListView.view.width
            }
            Rectangle {
                id: background1
                z: -1
                anchors.fill: parent
                focus: true
                LingmoUI.Theme.colorSet: LingmoUI.Theme.View
                color: LingmoUI.Theme.backgroundColor
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
                        // Reset top margin from LingmoComponents.ListItem
                        topMargin: -Math.round(LingmoUI.Units.gridUnit / 3)
                    }
                    height: Math.max(LingmoUI.Units.iconSizes.medium, connectionNameLabel.height + connectionDescriptionLabel.height) + Math.round(LingmoUI.Units.gridUnit / 2)

                    LingmoUI.Icon {
                        id: connectionIcon

                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                        height: LingmoUI.Units.iconSizes.medium; width: height
                        source: ConnectionIcon
                    }

                    Text {
                        id: connectionNameLabel

                        anchors {
                            bottom: ConnectionType === LingmoNM.Enums.Vpn ? connectionIcon.verticalCenter : undefined
                            left: connectionIcon.right
                            leftMargin: Math.round(LingmoUI.Units.gridUnit / 2)
                            right: parent.right
                            verticalCenter: ConnectionType === LingmoNM.Enums.Vpn ? undefined : parent.verticalCenter
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
                            leftMargin: Math.round(LingmoUI.Units.gridUnit / 2)
                            right: parent.right
                            top: connectionNameLabel.bottom
                        }
                        color: textColor
                        height: visible ? paintedHeight : 0
                        elide: Text.ElideRight
                        font.pointSize: LingmoUI.Theme.smallFont.pointSize
                        opacity: 0.6
                        text: ConnectionDescription
                        visible: ConnectionType === LingmoNM.Enums.Vpn
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
            margins: Math.round(LingmoUI.Units.gridUnit / 2)
        }
        spacing: LingmoUI.Units.mediumSpacing

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
            margins: Math.round(LingmoUI.Units.gridUnit / 2)
        }
        icon.name: "configure"

        QQC2.ToolTip.text: i18n("Configuration")
        QQC2.ToolTip.visible: hovered

        onClicked: {
            dialog.configurationDialogRequested();
        }
    }
}
