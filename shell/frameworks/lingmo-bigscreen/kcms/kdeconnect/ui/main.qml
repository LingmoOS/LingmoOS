/*
    SPDX-FileCopyrightText: 2020 Aditya Mehra <aix.m@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

*/

import QtQuick.Layouts 1.14
import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.14
import org.kde.lingmo.core 2.0 as LingmoCore
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.kdeconnect 1.0
import org.kde.kcmutils as KCM
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import "delegates" as Delegates

KCM.SimpleKCM {
    id: root
    
    title: i18n("KDE Connect")
    background: null
    leftPadding: LingmoUI.Units.smallSpacing
    topPadding: 0
    rightPadding: LingmoUI.Units.smallSpacing
    bottomPadding: 0
    
    Component.onCompleted: {
        connectionView.forceActiveFocus();
    }

    DevicesModel {
        id: allDevicesModel
    }

    contentItem: FocusScope {

        Rectangle {
            id: headerAreaTop
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: -LingmoUI.Units.largeSpacing
            anchors.rightMargin: -LingmoUI.Units.largeSpacing
            height: parent.height * 0.075
            z: 10
            gradient: Gradient {
                GradientStop { position: 0.1; color: Qt.rgba(0, 0, 0, 0.5) }
                GradientStop { position: 0.9; color: Qt.rgba(0, 0, 0, 0.25) }
            }

            LingmoUI.Heading {
                level: 1
                anchors.fill: parent
                anchors.topMargin: LingmoUI.Units.largeSpacing
                anchors.leftMargin: LingmoUI.Units.largeSpacing * 2
                anchors.bottomMargin: LingmoUI.Units.largeSpacing
                color: LingmoUI.Theme.textColor
                text: "KDE Connect"
            }
        }

        Item {
            id: footerMain
            anchors.left: parent.left
            anchors.right: deviceConnectionView.left
            anchors.leftMargin: -LingmoUI.Units.largeSpacing
            anchors.bottom: parent.bottom
            implicitHeight: LingmoUI.Units.gridUnit * 2

            Button {
                id: kcmcloseButton
                implicitHeight: LingmoUI.Units.gridUnit * 2
                width: allDevicesModel.count > 0 ? parent.width : (root.width + LingmoUI.Units.largeSpacing)

                background: Rectangle {
                    color: kcmcloseButton.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
                }

                contentItem: Item {
                    RowLayout {
                        anchors.centerIn: parent
                        LingmoUI.Icon {
                            Layout.preferredWidth: LingmoCore.Units.iconSizes.small
                            Layout.preferredHeight: LingmoCore.Units.iconSizes.small
                            source: "window-close"
                        }
                        Label {
                            text: i18n("Exit")
                        }
                    }
                }

                Keys.onUpPressed: connectionView.forceActiveFocus()

                onClicked: {
                    Window.window.close()
                }

                Keys.onReturnPressed: {
                    Window.window.close()
                }
            }
        }

        Item {
            clip: true
            anchors.left: parent.left
            anchors.top: headerAreaTop.bottom
            anchors.bottom: footerMain.top
            width: parent.width - deviceConnectionView.width

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: LingmoUI.Units.largeSpacing
                anchors.topMargin: LingmoUI.Units.largeSpacing * 2

                BigScreen.TileView {
                    id: connectionView
                    focus: true
                    model:  allDevicesModel
                    Layout.alignment: Qt.AlignTop
                    title: allDevicesModel.count > 0 ? "Found Devices" : "No Devices Found"
                    currentIndex: 0
                    delegate: Delegates.DeviceDelegate{}
                    navigationDown: kcmcloseButton
                    Behavior on x {
                        NumberAnimation {
                            duration: LingmoUI.Units.longDuration * 2
                            easing.type: Easing.InOutQuad
                        }
                    }
                    onCurrentItemChanged: {
                        deviceConnectionView.currentDevice = currentItem.deviceObj
                    }
                }
            }
        }

        DeviceConnectionView {
            id: deviceConnectionView
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            visible: allDevicesModel.count > 0 ? 1 : 0
            anchors.rightMargin: -LingmoUI.Units.smallSpacing
            width: LingmoUI.Units.gridUnit * 15
        }
    }
}
