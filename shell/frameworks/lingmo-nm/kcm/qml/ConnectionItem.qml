/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.1
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.5 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.networkmanagement as LingmoNM

ListItem {
    id: connectionItem

    height: connectionItemBase.height
    acceptedButtons: Qt.AllButtons
    checked: ConnectionPath === connectionView.currentConnectionPath

    signal aboutToChangeConnection(bool exportable, string name, string path)
    signal aboutToRemoveConnection(string name, string path)

    onClicked: mouse => {
        if (mouse.button === Qt.LeftButton) {
            aboutToChangeConnection(KcmVpnConnectionExportable, Name, ConnectionPath)
        } else if (mouse.button === Qt.RightButton) {
            connectionItemMenu.popup()
        }
    }

    Item {
        id: connectionItemBase

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: -Math.round(LingmoUI.Units.gridUnit / 3)
        }
        height: Math.max(LingmoUI.Units.iconSizes.medium, connectionNameLabel.height + connectionStatusLabel.height) + Math.round(LingmoUI.Units.gridUnit / 2)

        LingmoUI.Icon {
            id: connectionIcon

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
            color: (connectionItem.checked || connectionItem.pressed) ? LingmoUI.Theme.highlightedTextColor : textColor
            height: LingmoUI.Units.iconSizes.medium; width: height
            source: KcmConnectionIcon
        }

        Text {
            id: connectionNameLabel

            anchors {
                bottom: connectionIcon.verticalCenter
                left: connectionIcon.right
                leftMargin: Math.round(LingmoUI.Units.gridUnit / 2)
                right: connectingIndicator.visible ? connectingIndicator.left : parent.right
            }
            color: (connectionItem.checked || connectionItem.pressed) ? LingmoUI.Theme.highlightedTextColor : textColor
            height: paintedHeight
            elide: Text.ElideRight
            font.weight: ConnectionState === LingmoNM.Enums.Activated ? Font.DemiBold : Font.Normal
            font.italic: ConnectionState === LingmoNM.Enums.Activating ? true : false
            text: Name
            textFormat: Text.PlainText
        }

        Text {
            id: connectionStatusLabel

            anchors {
                left: connectionIcon.right
                leftMargin: Math.round(LingmoUI.Units.gridUnit / 2)
                right: connectingIndicator.visible ? connectingIndicator.left : parent.right
                top: connectionNameLabel.bottom
            }
            color: (connectionItem.checked || connectionItem.pressed) ? LingmoUI.Theme.highlightedTextColor : textColor
            height: paintedHeight
            elide: Text.ElideRight
            font.pointSize: LingmoUI.Theme.smallFont.pointSize
            opacity: 0.6
            text: itemText()
        }

        LingmoComponents3.BusyIndicator {
            id: connectingIndicator

            anchors {
                right: parent.right
                rightMargin: Math.round(LingmoUI.Units.gridUnit / 2)
                verticalCenter: connectionIcon.verticalCenter
            }
            height: LingmoUI.Units.iconSizes.medium
            width: LingmoUI.Units.iconSizes.medium
            running: ConnectionState === LingmoNM.Enums.Activating
            visible: running
        }
    }

    QQC2.Menu {
        id: connectionItemMenu

        QQC2.MenuItem {
            text: ConnectionState === LingmoNM.Enums.Deactivated ? i18n("Connect") : i18n("Disconnect")
            visible: ItemType === 1
            onTriggered: {
                if (ConnectionState === LingmoNM.Enums.Deactivated) {
                    handler.activateConnection(ConnectionPath, DevicePath, SpecificPath);
                } else {
                    handler.deactivateConnection(ConnectionPath, DevicePath);
                }
            }
        }

        QQC2.MenuItem {
            icon.name: "list-remove"
            text: i18n("Delete");

            onTriggered: {
                aboutToRemoveConnection(Name, ConnectionPath)
            }
        }

        QQC2.MenuItem {
            icon.name: "document-export"
            visible: KcmVpnConnectionExportable
            text: i18n("Export");

            onTriggered: kcm.onRequestExportConnection(ConnectionPath)
        }
    }

    /* This generates the status description under each connection
       in the list at the left side of the applet. */
    function itemText() {
        if (ConnectionState === LingmoNM.Enums.Activated) {
            return i18n("Connected")
        } else if (ConnectionState === LingmoNM.Enums.Activating) {
            return i18n("Connecting")
        } else {
            return LastUsed
        }
    }
}
