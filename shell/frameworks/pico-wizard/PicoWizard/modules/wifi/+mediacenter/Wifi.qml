// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.9 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents

import PicoWizard 1.0

ModuleMediaCenter {
    id: root
    property var activeFocusedElement

    function switchTextByFocus(){
        switch(root.activeFocusedElement) {
            case "wifiContainer":
                return [qsTr('Remote: Press the "Select|OK" button to start selecting a wireless network from the list'), qsTr('Keyboard: Press the "Enter" button to start selecting a wireless network from the list')]
            case "wifiSkipButton":
                return [qsTr('Remote: Press the "Select|OK" button to skip wireless setup and continue'), qsTr('Keyboard: Press the "Enter" button to skip wireless setup and continue')]
            case "wifiListView":
                return [qsTr('Remote: Use "Up|Down" buttons to navigate, "Select|OK" button to select a connection'), qsTr('Keyboard: Use "Up|Down" buttons to navigate, "Enter" button to select a connection')]
        }
    }

    delegate: Item {
        id: delegateRoot

        Component.onCompleted: {
            wifiContainer.forceActiveFocus()
        }

        ColumnLayout {
            anchors {
                top: parent.top
                bottom: parent.bottom
                bottomMargin: 16
                horizontalCenter: parent.horizontalCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    color: Kirigami.Theme.highlightColor
                    radius: 4
                    width: Kirigami.Units.gridUnit * 6
                    height: Kirigami.Units.gridUnit * 1
                    anchors.top: parent.top
                    x: infoRectContent.x
                    y: -2
                    z: 2

                    Label {
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        minimumPixelSize: 2
                        font.pixelSize: 72
                        maximumLineCount: 3
                        fontSizeMode: Text.Fit
                        wrapMode: Text.WordWrap
                        text: qsTr("Setting Up Your WiFi")
                        color: Kirigami.Theme.textColor
                    }
                }

                Rectangle {
                    id: infoRectContent
                    color: Kirigami.Theme.backgroundColor
                    radius: 4
                    width: parent.width
                    height: Kirigami.Units.gridUnit * 5
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: Kirigami.Units.smallSpacing
                    z: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.largeSpacing

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Item {
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                                Layout.fillHeight: true

                                Kirigami.Icon {
                                    anchors.fill: parent
                                    source: wifiModule.dir() + "/assets/remote-ok.svg"
                                }
                            }

                            Kirigami.Separator {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                            }

                            Label {
                                id: labelButtonInfo
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.weight: Font.Light
                                minimumPixelSize: 2
                                font.pixelSize: 25
                                maximumLineCount: 2
                                fontSizeMode: Text.Fit
                                wrapMode: Text.WordWrap
                                text: switchTextByFocus()[0]
                                color: Kirigami.Theme.textColor
                            }
                        }

                        Kirigami.Separator {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Item {
                                Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                                Layout.fillHeight: true

                                Kirigami.Icon {
                                    anchors.fill: parent
                                    source: wifiModule.dir() + "/assets/keyboard-ok.svg"
                                }
                            }

                            Kirigami.Separator {
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                            }

                            Label {
                                id: labelButtonInfo2
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                font.weight: Font.Light
                                minimumPixelSize: 2
                                font.pixelSize: 25
                                maximumLineCount: 2
                                fontSizeMode: Text.Fit
                                wrapMode: Text.WordWrap
                                text: switchTextByFocus()[1]
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: wifiContainer
                objectName: "wifiContainer"
                Layout.preferredWidth: root.width * 0.7
                Layout.fillHeight: true

                radius: 4
                border.width: wifiContainer.activeFocus ? 3 : 1
                border.color: wifiContainer.activeFocus ? Kirigami.Theme.highlightColor : Qt.lighter(Kirigami.Theme.backgroundColor, 1.5)
                color: Kirigami.Theme.backgroundColor

                KeyNavigation.down: skipButton
                Keys.onReturnPressed: {
                    wifiListView.forceActiveFocus()
                }

                onActiveFocusChanged: {
                    if(activeFocus) {
                        root.activeFocusedElement = objectName
                    }
                }

                Kirigami.CardsListView {
                    id: wifiListView
                    objectName: "wifiListView"
                    anchors.fill: parent
                    anchors.margins: 8

                    spacing: 4
                    model: wifiModule.model
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        active: true
                    }
                    keyNavigationEnabled: true
                    highlightFollowsCurrentItem: true

                    onActiveFocusChanged: {
                        if(activeFocus) {
                            root.activeFocusedElement = objectName
                        }
                    }

                    delegate: Kirigami.AbstractCard {
                        width: parent ? parent.width : 0
                        height: 60
                        showClickFeedback: true
                        highlighted: !wifiContainer.activeFocus && !skipButton.activeFocus && !backButton.activeFocus && wifiListView.currentIndex == index

                        Rectangle {
                            anchors.fill: parent
                            color: wifiListView.currentIndex == index ? Kirigami.Theme.highlightColor : "transparent"
                            border.width: 0
                            radius: parent.background.radius

                            RowLayout {
                                anchors {
                                    verticalCenter: parent.verticalCenter
                                    left: parent.left
                                    leftMargin: 12
                                }

                                Kirigami.Icon {
                                    Layout.leftMargin: 0
                                    Layout.rightMargin: 4
                                    Layout.preferredWidth: 24
                                    Layout.preferredHeight: 24
                                    opacity: 0.7
                                    source: wifiModule.getWifiIcon(signal)
                                    color: Kirigami.Theme.textColor
                                }

                                Label {
                                    id: wifiName
                                    color: Kirigami.Theme.textColor
                                    text: trimName(ssid)
                                    font.pointSize: 10

                                    function trimName(name) {
                                        if (name.length > 18) {
                                            return name.slice(0, 18) + "..."
                                        } else {
                                            return name
                                        }
                                    }
                                }
                            }

                            RowLayout {
                                anchors {
                                    right: parent.right
                                    verticalCenter: parent.verticalCenter
                                    rightMargin: 12
                                }
                                Kirigami.Icon {
                                    visible: isSecured
                                    opacity: 0.7
                                    Layout.leftMargin: 0
                                    Layout.rightMargin: 0
                                    Layout.preferredWidth: 10
                                    Layout.preferredHeight: 10
                                    source: "lock"
                                    color: Kirigami.Theme.textColor
                                }

                                Label {
                                    text: security
                                    color: Kirigami.Theme.textColor
                                    opacity: 0.5
                                    font.pointSize: 7
                                }
                            }
                        }

                        Keys.onReturnPressed: {
                            clicked()
                        }

                        Keys.onEscapePressed: {
                            wifiContainer.forceActiveFocus()
                        }

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

                ColumnLayout {
                    anchors.centerIn: wifiListView
                    Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter
                    spacing: 12
                    visible: wifiListView.count <= 0

                    Kirigami.Icon {
                        width: 18
                        height: 18
                        Layout.alignment: Qt.AlignHCenter|Qt.AlignVCenter

                        source: wifiModule.dir() + "/assets/spinner.svg"
                        color: Kirigami.Theme.textColor
                        opacity: 0.5

                        RotationAnimation on rotation {
                            loops: Animation.Infinite
                            from: -90
                            to: 270
                            duration: 500
                            running: true
                        }
                    }

                    Label {
                        font.italic: true
                        text: qsTr("Listing Wifi Connections")
                        color: Kirigami.Theme.textColor
                        opacity: 0.5
                    }
                }
            }

            RowLayout {
                Layout.preferredWidth: root.width * 0.7
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: backButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    radius: 3

                    Button {
                        id: backButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: backButton.activeFocus ? 1 : 0
                        KeyNavigation.right: skipButton

                        icon.name: "go-previous"
                        text: "Back"

                        Keys.onReturnPressed: clicked()
                        KeyNavigation.up: wifiContainer

                        onClicked: {
                            moduleLoader.back()
                        }
                        visible: moduleLoader.hasPrevious
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                    color: "transparent"
                    border.width: skipButton.activeFocus ? 3 : 0
                    border.color: Kirigami.Theme.highlightColor
                    radius: 3

                    Button {
                        id: skipButton
                        anchors.fill: parent
                        anchors.margins: 2
                        highlighted: skipButton.activeFocus ? 1 : 0
                        objectName: "wifiSkipButton"

                        KeyNavigation.left: backButton
                        KeyNavigation.up: wifiContainer

                        icon.name: "go-next-skip"
                        text: "Skip"

                        visible: !hideSkip

                        Keys.onReturnPressed: clicked()

                        onClicked: {
                            moduleLoader.nextModule()
                        }

                        onActiveFocusChanged: {
                            if(activeFocus) {
                                root.activeFocusedElement = objectName
                            }
                        }
                    }
                }
            }
        }

        Dialog {
            id: passwordDialog
            property string wifiName
            closePolicy: Popup.CloseOnEscape
            dim: true
            modal: true
            z: 10
            font.pixelSize: 10
            parent: Overlay.overlay
            implicitWidth: Kirigami.Units.gridUnit * 30

            x: Math.round((parent.width - width) / 2)
            y: parent.height * 0.10

            onVisibleChanged: {
                if (visible) {
                    password.forceActiveFocus();
                } else {
                    wifiListView.forceActiveFocus();
                }
            }

            contentItem: ColumnLayout {
                Keys.onEscapePressed: passwordDialog.close()

                Kirigami.Heading {
                    text: qsTr("Connect to %1").arg(trimName(passwordDialog.wifiName))
                    color: Kirigami.Theme.textColor
                    level: 1
                    font.bold: true
                    topPadding: Kirigami.Units.smallSpacing
                    bottomPadding: Kirigami.Units.smallSpacing

                    function trimName(name) {
                        if (name.length > 15) {
                            return name.slice(0, 15) + "..."
                        } else {
                            return name
                        }
                    }
                }

                Kirigami.PasswordField {
                    id: password
                    Layout.fillWidth: true
                    topPadding: 16
                    bottomPadding: 16
                    Layout.topMargin: 16
                    placeholderText: "Enter Password"
                    KeyNavigation.down: connectButton.enabled ? connectButton : closeButton

                    onAccepted: {
                        wifiModule.setWifi(wifiListView.currentIndex, password.text)
                        passwordDialog.close()
                        connectingPopup.open()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Button {
                        id: connectButton
                        Layout.fillWidth:true
                        enabled: password.text.length > 0
                        text: "Connect"
                        icon.name: "network-connect"
                        hoverEnabled: true
                        KeyNavigation.up: password
                        KeyNavigation.right: closeButton

                        Keys.onReturnPressed: clicked()
                        onClicked: {
                            wifiModule.setWifi(wifiListView.currentIndex, password.text)
                            passwordDialog.close()
                            connectingPopup.open()
                        }
                    }

                    Button {
                        id: closeButton
                        Layout.fillWidth:true
                        text: "Cancel"
                        icon.name: "dialog-close"
                        hoverEnabled: true
                        KeyNavigation.up: password
                        KeyNavigation.left: connectButton

                        Keys.onReturnPressed: clicked()
                        onClicked: {
                            passwordDialog.close()
                        }
                    }
                }
            }
        }

        Popup {
            id: connectingPopup
            modal: true
            implicitWidth: Kirigami.Units.gridUnit * 30
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

                Kirigami.Icon {
                    width: 18
                    height: 18

                    source: wifiModule.dir() + "/assets/spinner.svg"
                    color: Kirigami.Theme.textColor
                    opacity: 0.5

                    RotationAnimation on rotation {
                        loops: Animation.Infinite
                        from: -90
                        to: 270
                        duration: 500
                        running: true
                    }
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

                if (strength > 0 && strength < 25) {
                    icon = "network-wireless-connected-25"
                } else if (strength >= 25 && strength < 50) {
                    icon = "network-wireless-connected-50"
                } else if (strength >= 50 && strength < 75) {
                    icon = "network-wireless-connected-75"
                } else if (strength >= 75) {
                    icon = "network-wireless-connected-100"
                } else {
                    icon = "network-wireless-connected-00"
                }

                return icon
            }

            Component.onCompleted: {
                root.moduleName = "WiFi Setup"
                root.moduleIcon = wifiModule.dir() + "/assets/wifi.svg"
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
