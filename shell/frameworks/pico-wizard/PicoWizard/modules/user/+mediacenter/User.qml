// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.9 as Kirigami

import PicoWizard 1.0

ModuleMediaCenter {
    id: root
    moduleName: userModule.moduleName()
    moduleIcon: userModule.dir() + "/assets/user.svg"
    hideSkip: true
    property var digitValidator: RegExpValidator { regExp: /[0-9]*/ }
    property var alphaNumericValidator: RegExpValidator { regExp: /.*/ }
    property var activeFocusedElement

    function switchTextByFocus(){
        switch(root.activeFocusedElement) {
            case "fullNameContainer":
                return [qsTr('Remote: Press the "Select|OK" button to enter your full name details'), qsTr('Keyboard: Press the "Enter" button to enter your full name details')]
            case "userNameContainer":
                return [qsTr('Remote: Press the "Select|OK" button to enter your username details'), qsTr('Keyboard: Press the "Enter" button to enter your username details')]
            case "passwordContainer":
                return [qsTr('Remote: Press the "Select|OK" button to enter your secure password details'), qsTr('Keyboard: Press the "Enter" button to enter your secure password details')]
            case "cnfPasswordContainer":
                return [qsTr('Remote: Press the "Select|OK" button to enter confirm password details'), qsTr('Keyboard: Press the "Enter" button to confirm password details')]
            case "nextButton":
                return [qsTr('Remote: Press the "Select|OK" button to continue'), qsTr('Keyboard: Press the "Enter" button to continue')]
        }
    }

    delegate: Item {

        Component.onCompleted: {
            fullNameContainer.forceActiveFocus()
        }

        ColumnLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.width * 0.7
            spacing: Kirigami.Units.smallSpacing

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 6
                Layout.alignment: Qt.AlignHCenter

                Rectangle {
                    color: Kirigami.Theme.highlightColor
                    radius: 4
                    width: Kirigami.Units.gridUnit * 8
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
                        text: qsTr("Setting Up Your User Account")
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
                                    source: userModule.dir() + "/assets/remote-ok.svg"
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
                                text: root.switchTextByFocus()[0]
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
                                    source: userModule.dir() + "/assets/keyboard-ok.svg"
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
                                text: root.switchTextByFocus()[1]
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: fullNameContainer
                objectName: "fullNameContainer"
                color: "transparent"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2.25
                border.color: fullNameContainer.activeFocus ? Kirigami.Theme.highlightColor : "transparent"
                border.width: fullNameContainer.activeFocus ? 3 : 0
                Layout.alignment: Qt.AlignTop

                Keys.onReturnPressed: fullname.forceActiveFocus()
                KeyNavigation.down: userNameContainer
                KeyNavigation.up: nextButton.enabled ? nextButton : backButton

                onActiveFocusChanged: {
                    if(activeFocus) {
                        root.activeFocusedElement = objectName
                    }
                }

                PlasmaComponents.TextField {
                    id: fullname
                    anchors.fill: parent
                    anchors.margins: 3
                    placeholderText: qsTr("Full Name")
                    topPadding: 16
                    bottomPadding: 16
                }
            }

            Rectangle {
                id: userNameContainer
                objectName: "userNameContainer"
                color: "transparent"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2.25
                Layout.alignment: Qt.AlignTop
                border.color: userNameContainer.activeFocus ? Kirigami.Theme.highlightColor : "transparent"
                border.width: userNameContainer.activeFocus ? 3 : 0

                Keys.onReturnPressed: username.forceActiveFocus()
                KeyNavigation.up: fullNameContainer
                KeyNavigation.down: passwordContainer

                onActiveFocusChanged: {
                    if(activeFocus) {
                        root.activeFocusedElement = objectName
                    }
                }

                PlasmaComponents.TextField {
                    id: username
                    validator: RegExpValidator { regExp: /[a-z_][a-z0-9_-]*[$]?/ }
                    maximumLength: 32
                    anchors.fill: parent
                    anchors.margins: 3
                    topPadding: 16
                    bottomPadding: 16
                    inputMethodHints: Qt.ImhLowercaseOnly | Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                    placeholderText: qsTr("Username")

                    Popup {
                        id: usernamePopup
                        y: -(height+5)
                        width: parent.width
                        visible: username.focus

                        closePolicy: Popup.CloseOnPressOutsideParent

                        background: Rectangle {
                            radius: 2
                            color: Kirigami.Theme.backgroundColor
                        }

                        ColumnLayout {
                            anchors.fill: parent

                            Label {
                                text: "- Should be lowercase"
                                color: Kirigami.Theme.textColor
                            }
                            Label {
                                text: "- Should start with [a-z] or '_'"
                                color: Kirigami.Theme.textColor
                            }
                            Label {
                                text: "- Can contain [a-z], [0-9], '_', and '-'"
                                color: Kirigami.Theme.textColor
                            }
                            Label {
                                text: "- May end with a '$'"
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: passwordContainer
                objectName: "passwordContainer"
                color: "transparent"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2.25
                border.color: passwordContainer.activeFocus ? Kirigami.Theme.highlightColor : "transparent"
                border.width: passwordContainer.activeFocus ? 3 : 0
                Layout.alignment: Qt.AlignTop

                Keys.onReturnPressed: password.forceActiveFocus()
                KeyNavigation.up: userNameContainer
                KeyNavigation.down: cnfPasswordContainer

                onActiveFocusChanged: {
                    if(activeFocus) {
                        root.activeFocusedElement = objectName
                    }
                }

                PlasmaComponents.TextField {
                    id: password
                    validator: {
                        if (userModule.passwordType === 'digitsonly')
                            return digitValidator
                        else if (userModule.passwordType === 'alphanumeric')
                            return alphaNumericValidator
                    }

                    anchors.fill: parent
                    anchors.margins: 3
                    topPadding: 16
                    bottomPadding: 16
                    passwordCharacter: "*"
                    revealPasswordButtonShown: true
                    echoMode: "Password"
                    placeholderText: userModule.passwordType === 'digitsonly' ? qsTr("Pin") : qsTr("Password")
                    inputMethodHints: userModule.passwordType === 'digitsonly' ? Qt.ImhDigitsOnly : Qt.ImhNoAutoUppercase | Qt.ImhNoPredectiveText

                    Popup {
                        y: -(height+5)
                        width: parent.width
                        visible: password.focus && userModule.passwordType === 'digitsonly'
                        closePolicy: Popup.CloseOnPressOutsideParent

                        background: Rectangle {
                            radius: 2
                            color: Kirigami.Theme.backgroundColor
                        }

                        ColumnLayout {
                            anchors.fill: parent

                            Label {
                                text: "- Should be digits only [0-9]"
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }
                }
            }

            Rectangle {
                id: cnfPasswordContainer
                objectName: "cnfPasswordContainer"
                color: "transparent"
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2.25
                border.color: cnfPasswordContainer.activeFocus ? Kirigami.Theme.highlightColor : "transparent"
                border.width: cnfPasswordContainer.activeFocus ? 3 : 0
                Layout.alignment: Qt.AlignTop

                onActiveFocusChanged: {
                    if(activeFocus) {
                        root.activeFocusedElement = objectName
                    }
                }

                Keys.onReturnPressed: cnfPassword.forceActiveFocus()
                KeyNavigation.up: passwordContainer
                KeyNavigation.down: nextButton.enabled ? nextButton : backButton

                PlasmaComponents.TextField {
                    id: cnfPassword
                    validator: {
                        if (userModule.passwordType === 'digitsonly')
                            return digitValidator
                        else if (userModule.passwordType === 'alphanumeric')
                            return alphaNumericValidator
                    }

                    anchors.fill: parent
                    anchors.margins: 3
                    topPadding: 16
                    bottomPadding: 16
                    passwordCharacter: "*"
                    revealPasswordButtonShown: true
                    echoMode: "Password"
                    placeholderText: userModule.passwordType === 'digitsonly' ? qsTr("Confirm Pin") : qsTr("Confirm Password")
                    inputMethodHints: userModule.passwordType === 'digitsonly' ? Qt.ImhDigitsOnly : Qt.ImhNoAutoUppercase | Qt.ImhNoPredectiveText
                }
            }
        }

        RowLayout {
            width: root.width * 0.7
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: 0
            }

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

                    KeyNavigation.right: nextButton

                    icon.name: "go-previous"
                    text: "Back"

                    Keys.onReturnPressed: clicked()

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
                border.width: nextButton.activeFocus ? 3 : 0
                border.color: Kirigami.Theme.highlightColor
                radius: 3

                NextButtonMediaCenter {
                    id: nextButton
                    objectName: "nextButton"
                    anchors.fill: parent
                    anchors.margins: 2
                    highlighted: nextButton.activeFocus ? 1 : 0
                    enabled: fullname.text.length > 0 && username.text.length > 0 && cnfPassword.text.length > 0 && cnfPassword.text === password.text ? 1 : 0

                    KeyNavigation.left: backButton

                    onNextClicked: {
                        accepted = true
                        userModule.createUser(fullname.text, username.text, cnfPassword.text)
                    }
                }
            }
        }

        UserModule {
            id: userModule

            Component.onCompleted: {
                root.moduleName = "User Setup"
                root.moduleIcon = userModule.dir() + "/assets/user.svg"
            }

            property var signals: Connections {
                function onCreateUserSuccess() {
                    nextButton.next()
                }

                function onCreateUserFailed() {
                    nextButton.cancel()
                }

                function onErrorOccurred(err) {
                    applicationWindow().showPassiveNotification(err, 2000)
                }
            }
        }
    }
}
