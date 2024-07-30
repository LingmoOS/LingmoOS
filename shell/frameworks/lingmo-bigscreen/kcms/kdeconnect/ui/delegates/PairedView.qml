/*
    SPDX-FileCopyrightText: 2020 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.kdeconnect 1.0

Item {
    id: trustedDevice
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    onActiveFocusChanged: {
        unpairBtn.forceActiveFocus()
    }

    ColumnLayout {
        anchors.fill: parent
        
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: i18n("This device is paired")
        }
        
        Button {
            id: unpairBtn
            Layout.fillWidth: true
            Layout.preferredHeight: LingmoUI.Units.gridUnit * 2
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
            
            KeyNavigation.up: backBtnSettingsItem
            
            Keys.onReturnPressed: {
                clicked()
            }
            
            onClicked: deviceView.currentDevice.unpair()
            
            background: Rectangle {
                color: unpairBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                border.width: 0.75
                border.color: Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r, LingmoUI.Theme.backgroundColor.g, LingmoUI.Theme.backgroundColor.b, 0.8))
            }
            
            contentItem: Item {
                RowLayout {
                    anchors.centerIn: parent
                
                    LingmoUI.Icon {
                        Layout.preferredWidth: LingmoCore.Units.iconSizes.small
                        Layout.preferredHeight: LingmoCore.Units.iconSizes.small
                        source: "network-disconnect"
                    }
                    LingmoComponents.Label {
                        text: i18n("Unpair")
                    }
                }
            }
        }
    }
} 
