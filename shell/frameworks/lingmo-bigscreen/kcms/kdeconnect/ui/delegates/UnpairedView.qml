/*
    SPDX-FileCopyrightText: 2020 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.kdeconnect 1.0

Item {
    id: untrustedDevice
    Layout.fillWidth: true
    Layout.fillHeight: true
    
    onActiveFocusChanged: {
        pairBtn.forceActiveFocus()
    }
    
    Timer {
           id: timer
    }

    function delay(delayTime, cb) {
            timer.interval = delayTime;
            timer.repeat = false;
            timer.triggered.connect(cb);
            timer.start();
    }
    
    ColumnLayout {
        anchors.fill: parent
        
        LingmoComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: i18n("This device is not paired")
        }
        
        Button {
            id: pairBtn
            Layout.fillWidth: true
            Layout.preferredHeight: LingmoUI.Units.gridUnit * 2
            LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
            
            Keys.onReturnPressed: {
                clicked()
            }
                
            onClicked: {
                deviceView.currentDevice.requestPair()
                pairRequestNotification.visible = true
                delay(2500, function() {
                    pairRequestNotification.visible = false
                })
            }
                
            KeyNavigation.up: backBtnSettingsItem
        
            background: Rectangle {
                color: pairBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                border.width: 0.75
                border.color: Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r, LingmoUI.Theme.backgroundColor.g, LingmoUI.Theme.backgroundColor.b, 0.8))
            }
            
            contentItem: Item {
                RowLayout {
                    anchors.centerIn: parent
                
                    LingmoUI.Icon {
                        Layout.preferredWidth: LingmoCore.Units.iconSizes.small
                        Layout.preferredHeight: LingmoCore.Units.iconSizes.small
                        source: "network-connect"
                    }
                    
                    LingmoComponents.Label {
                        text: i18n("Pair")
                    }
                }
            }
        }
        
        LingmoComponents.Label {
            id: pairRequestNotification
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            visible: false
            text: i18n("Pairing request sent to device")
        }
    }
} 
