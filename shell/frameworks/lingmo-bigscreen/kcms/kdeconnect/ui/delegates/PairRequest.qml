/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.kdeconnect 1.0

Item {
    id: pairDevice
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    onActiveFocusChanged: {
        acceptBtn.forceActiveFocus()
    }

    ColumnLayout {
        anchors.fill: parent
        
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: i18n("This device is requesting to be paired")
        }
        
        Button {
            id: acceptBtn
            Layout.fillWidth: true
            Layout.preferredHeight: LingmoUI.Units.gridUnit * 2
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
            
            KeyNavigation.up: backBtnSettingsItem
            KeyNavigation.down: rejectBtn
            
            Keys.onReturnPressed: {
                clicked()
            }
            
            onClicked: deviceView.currentDevice.acceptPairing()
            
            background: Rectangle {
                color: acceptBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                border.width: 0.75
                border.color: Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r, LingmoUI.Theme.backgroundColor.g, LingmoUI.Theme.backgroundColor.b, 0.8))
            }
            
            contentItem: Item {
                RowLayout {
                    anchors.centerIn: parent
                
                    LingmoUI.Icon {
                        Layout.preferredWidth: LingmoCore.Units.iconSizes.small
                        Layout.preferredHeight: LingmoCore.Units.iconSizes.small
                        source: "dialog-ok"
                    }
                    LingmoComponents.Label {
                        text: i18n("Accept")
                    }
                }
            }
        }
        
        Button {
            id: rejectBtn
            Layout.fillWidth: true
            Layout.preferredHeight: LingmoUI.Units.gridUnit * 2
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
            
            KeyNavigation.up: acceptBtn
            
            Keys.onReturnPressed: {
                clicked()
            }
            
            onClicked: deviceView.currentDevice.rejectPairing()
            
            background: Rectangle {
                color: rejectBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                border.width: 0.75
                border.color: Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r, LingmoUI.Theme.backgroundColor.g, LingmoUI.Theme.backgroundColor.b, 0.8))
            }
            
            contentItem: Item {
                RowLayout {
                    anchors.centerIn: parent
                
                    LingmoUI.Icon {
                        Layout.preferredWidth: LingmoCore.Units.iconSizes.small
                        Layout.preferredHeight: LingmoCore.Units.iconSizes.small
                        source: "dialog-cancel"
                    }
                    LingmoComponents.Label {
                        text: i18n("Reject")
                    }
                }
            }
        }
    }
} 
