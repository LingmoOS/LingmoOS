/*
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root

    signal switchUserClicked()
    signal canceled()

    property alias notification: message.text
    property bool switchUserEnabled
    property bool capsLockOn

    implicitWidth: layoutItem.width + theme.mSize(theme.defaultFont).width * 4 + 12
    implicitHeight: layoutItem.height + 12

    anchors {
        fill: parent
        margins: 6
    }

    Column {
        id: layoutItem
        anchors.centerIn: parent
        spacing: theme.mSize(theme.defaultFont).height / 2

        PlasmaComponents.Label {
            id: message
            text: ""
            anchors.horizontalCenter: parent.horizontalCenter
            font.bold: true
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }
            opacity: text == "" ? 0 : 1
        }

        PlasmaComponents.Label {
            id: capsLockMessage
            text: i18n("Warning: Caps Lock on")
            anchors.horizontalCenter: parent.horizontalCenter
            opacity: capsLockOn ? 1 : 0
            height: capsLockOn ? paintedHeight : 0
            font.bold: true
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }
        }

        PlasmaComponents.Label {
            id: lockMessage
            text: kscreenlocker_userName.length == 0 ? i18nd("kscreenlocker_greet", "The session is locked") :
                                                       i18nd("kscreenlocker_greet", "The session has been locked by %1", kscreenlocker_userName)
            anchors.horizontalCenter: parent.horizontalCenter
        }

        RowLayout {
            anchors.horizontalCenter: parent.horizontalCenter
            PlasmaComponents.Label {
                text: i18nd("kscreenlocker_greet", "Password:")
            }
            PlasmaComponents.TextField {
                id: password
                enabled: !authenticator.busy
                echoMode: TextInput.Password
                focus: true
                Keys.onEnterPressed: authenticator.tryUnlock()
                Keys.onReturnPressed: authenticator.tryUnlock()
                Keys.onEscapePressed: password.text = ""
            }
        }

        PlasmaComponents.ButtonRow {
            id: buttonRow
            property bool showAccel: false
            exclusive: false
            spacing: theme.mSize(theme.defaultFont).width / 2
            anchors.horizontalCenter: parent.horizontalCenter

            AccelButton {
                id: switchUser
                label: i18nd("kscreenlocker_greet", "&Switch Users")
                iconSource: "fork"
                visible: switchUserEnabled
                onClicked: switchUserClicked()
            }

            AccelButton {
                id: unlock
                label: i18nd("kscreenlocker_greet", "Un&lock")
                iconSource: "object-unlocked"
                enabled: !authenticator.graceLocked
                onClicked: authenticator.tryUnlock()
            }
        }
    }

    Keys.onPressed: {
        const alt = event.modifiers & Qt.AltModifier;
        buttonRow.showAccel = alt;

        if (alt) {
            // focus munging is needed otherwise the greet (QWidget)
            // eats all the key events, even if root is added to forwardTo
            // qml property of greeter
            // greeter.focus = false;
            root.forceActiveFocus();

            for (const button of [switchUser, unlock]) {
                if (event.key == button.accelKey) {
                    buttonRow.showAccel = false;
                    button.clicked();
                    break;
                }
            }
        }
    }

    Keys.onReleased: {
        buttonRow.showAccel = event.modifiers & Qt.AltModifier;
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
                password.focus = true;
            }
        }
        function onInfoMessage(text) {
            root.notification = text;
        }
        function onErrorMessage(text) {
            root.notification = text;
        }
        function onPromptForSecret() {
            authenticator.respond(password.text);
        }
        function onSucceeded() {
            Qt.quit()
        }
    }
}
