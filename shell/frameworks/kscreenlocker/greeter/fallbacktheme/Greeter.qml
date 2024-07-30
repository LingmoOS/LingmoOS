/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kscreenlocker 1.0 as ScreenLocker

Item {
    id: root

    signal switchUserClicked()
    signal canceled()

    property alias notification: message.text
    property bool switchUserEnabled
    property bool capsLockOn

    function resetFocus() {
        password.forceActiveFocus();
    }

    implicitWidth: layoutItem.width + Kirigami.Units.largeSpacing * 2
    implicitHeight: layoutItem.height + Kirigami.Units.largeSpacing * 2

    ColumnLayout {
        id: layoutItem
        anchors.centerIn: parent
        spacing: Kirigami.Units.largeSpacing

        PlasmaComponents3.Label {
            id: message
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: ""
            font.bold: true

            visible: opacity > 0
            opacity: text == "" ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }
        }

        PlasmaComponents3.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: i18n("Warning: Caps Lock on")
            font.bold: true

            visible: opacity > 0
            opacity: root.capsLockOn ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                }
            }
        }

        PlasmaComponents3.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: kscreenlocker_userName.length == 0 ? i18nd("kscreenlocker_greet", "The session is locked") :
                                                       i18nd("kscreenlocker_greet", "The session has been locked by %1", kscreenlocker_userName)
        }
        PlasmaExtras.PasswordField {
            id: password
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Kirigami.Units.gridUnit * 15
            enabled: !authenticator.busy
            Keys.onEnterPressed: authenticator.startAuthenticating()
            Keys.onReturnPressed: authenticator.startAuthenticating()
            Keys.onEscapePressed: password.text = ""
        }

        PlasmaComponents3.Label {
            visible: authenticator.authenticatorTypes & ScreenLocker.Authenticator.Fingerprint
            text: i18nd("kscreenlocker_greet", "(or place your fingerprint on the reader)")
            Layout.fillWidth: true
        }

        PlasmaComponents3.Label {
            visible: authenticator.authenticatorTypes & ScreenLocker.Authenticator.Smartcard
            text: i18nd("kscreenlocker_greet", "(or use your smartcard)")
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing

            PlasmaComponents3.Button {
                text: i18nd("kscreenlocker_greet", "&Switch Users")
                icon.name: "system-switch-user"
                visible: root.switchUserEnabled
                onClicked: switchUserClicked()
            }

            PlasmaComponents3.Button {
                text: i18nd("kscreenlocker_greet", "Un&lock")
                icon.name: "unlock"
                enabled: !authenticator.graceLocked
                onClicked: authenticator.startAuthenticating()
            }
        }
    }

    Connections {
        target: authenticator

        function onFailed() {
            root.notification = i18nd("kscreenlocker_greet", "Unlocking failed");
        }
        function onBusyChanged() {
            if (!authenticator.busy) {
                root.notification = "";
                password.selectAll();
                root.resetFocus();
            }
        }
        function onInfoMessageChanged() {
            root.notification = Qt.binding(() => authenticator.infoMessage);
        }
        function onErrorMessageChanged() {
            root.notification = Qt.binding(() => authenticator.errorMessage);
        }
        function onPromptForSecretChanged() {
            authenticator.respond(password.text);
        }
        function onSucceeded() {
            Qt.quit()
        }
    }

    Component.onCompleted: {
        root.resetFocus();
    }
}
