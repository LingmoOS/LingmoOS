// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.9 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents

import PicoWizard 1.0

Module {
    id: root

    delegate: Item {
        id: delegateRoot

        Rectangle {
            id: wifiContainer
            width: root.width * 0.7
            anchors {
                top: parent.top
                bottom: parent.bottom
                bottomMargin: 16
                horizontalCenter: parent.horizontalCenter
            }

            radius: 4
            border.width: 2
            border.color: Kirigami.Theme.backgroundColor
            color: Qt.lighter(Kirigami.Theme.backgroundColor, 1.1)

            ScrollIndicator {
                id: wifiScroll
                width: 12
                anchors {
                    top: wifiContainer.top
                    right: wifiContainer.right
                    bottom: wifiContainer.bottom
                }
            }

            ListView {
                id: wifiListView
                anchors.fill: parent
                anchors.margins: 8

                spacing: 4
                model: wifiModule.model
                clip: true
                ScrollIndicator.vertical: wifiScroll

                delegate: Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    width: parent ? parent.width : 0
                    height: 50

                    RowLayout {
                        anchors {
                            verticalCenter: parent.verticalCenter
                            left: parent.left
                            leftMargin: 12
                        }

                        Kirigami.Icon {
                            Layout.leftMargin: 0
                            Layout.rightMargin: 4
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            opacity: 0.7
                            source: wifiModule.getWifiIcon(signal)
                        }

                        ColumnLayout {
                            Label {
                                id: wifiName
                                text: trimName(ssid)
                                font.pointSize: 10
                                color: Kirigami.Theme.textColor

                                function trimName(name) {
                                    if (name.length > 18) {
                                        return name.slice(0, 18) + "..."
                                    } else {
                                        return name
                                    }
                                }
                            }

                            RowLayout {
                                Kirigami.Icon {
                                    visible: isSecured
                                    opacity: 0.7
                                    Layout.leftMargin: 0
                                    Layout.rightMargin: 0
                                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                                    source: "object-locked"
                                }

                                Label {
                                    text: security
                                    color: Kirigami.Theme.textColor
                                    opacity: 0.5
                                    font.pointSize: 7
                                }
                            }
                        }
                    }

                    Kirigami.Icon {
                        width: Kirigami.Units.iconSizes.medium
                        height: Kirigami.Units.iconSizes.medium
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            rightMargin: 12
                        }
                        source: "go-next"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (isSecured) {
                                wifiListView.currentIndex = index
                                passwordDialog.wifiName = wifiName.text
                                password.text = ""
                                passwordDialog.open()
                            } else {
                                wifiModule.setWifi(index, '')
                                connectingPopup.open()
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                anchors.centerIn: wifiListView
                Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
                spacing: 12
                visible: wifiListView.count <= 0

                BusyIndicator {
                    width: Kirigami.Units.smallMedium
                    height: Kirigami.Units.smallMedium
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter

                    opacity: 0.5
                    running: true
                }

                Label {
                    font.italic: true
                    text: qsTr("Listing Wifi Connections")
                    color: Kirigami.Theme.textColor
                    opacity: 0.5
                }
            }
        }

        Dialog {
            property string wifiName

            id: passwordDialog
            modal: true
            implicitWidth: 300
            z: 10

            font.pixelSize: 10

            parent: Overlay.overlay

            x: Math.round((parent.width - width) / 2)
            y: Math.round((parent.height - height) / 2)

            footer: DialogButtonBox {
                Button {
                    enabled: password.text.length > 0
                    text: qsTr("Connect")
                    DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                    font.pixelSize: 10
                    font.bold: true
                    hoverEnabled: true

                    onClicked: {
                        wifiModule.setWifi(wifiListView.currentIndex, password.text)
                        connectingPopup.open()
                    }
                }
            }

            ColumnLayout {
                anchors.left: parent.left
                anchors.right: parent.right

                Label {
                    text: qsTr("Connect to %1").arg(trimName(passwordDialog.wifiName))
                    color: Kirigami.Theme.textColor
                    font.pointSize: 11
                    font.bold: true
                    topPadding: 8
                    bottomPadding: 8

                    function trimName(name) {
                        if (name.length > 15) {
                            return name.slice(0, 15) + "..."
                        } else {
                            return name
                        }
                    }
                }

                PlasmaComponents.TextField {
                    id: password
                    Layout.fillWidth: true
                    topPadding: 16
                    bottomPadding: 16
                    Layout.topMargin: 16
                    passwordCharacter: "*"
                    revealPasswordButtonShown: true
                    echoMode: "Password"
                    placeholderText: qsTr("Password")
                }
            }
        }

        Popup {
            id: connectingPopup
            modal: true
            implicitWidth: 300
            z: 10
            closePolicy: Popup.NoAutoClose

            font.pixelSize: 11

            parent: Overlay.overlay

            x: Math.round((parent.width - width) / 2)
            y: Math.round((parent.height - height) / 2)

            RowLayout {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                BusyIndicator {
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                    running: true
                    visible: true

                    opacity: 0.5
                }

                Label {
                    text: qsTr("Connecting")
                    color: Kirigami.Theme.textColor
                    opacity: 0.5
                }
            }
        }

        WifiModule {
            id: wifiModule

            function getWifiIcon(strength) {
                var icon = ""

                if (strength >= 0 && strength < 25) {
                    icon = "network-wireless-signal-weak"
                } else if (strength >= 25 && strength < 50) {
                    icon = "network-wireless-signal-ok"
                } else if (strength >= 50 && strength < 75) {
                    icon = "network-wireless-signal-good"
                } else if (strength >= 75) {
                    icon = "network-wireless-signal-excellent"
                } else {
                    icon = "network-wireless-signal-none"
                }

                return icon
            }

            Component.onCompleted: {
                root.moduleName = wifiModule.moduleName()
                root.moduleIcon = "network-wireless-signal"
            }

            property var signals: Connections {
                function onConnectWifiSuccess() {
                    connectingPopup.close()
                    moduleLoader.nextModule()
                }

                function onConnectWifiFailed() {
                    connectingPopup.close()
                }

                function onErrorOccurred(err) {
                    applicationWindow().showPassiveNotification(err, 2000)
                }
            }
        }
    }
}
