/*
SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.1
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: true
    ColumnLayout {
        anchors.fill: parent
        Label {
            id: message
            Connections {
                target: authenticator
                function onPromptForSecret() {
                    authenticator.respond(password.text)
                }
                function onSucceeded() {
                    message.text = "Authentication succeeded";
                }
                function onFailed() {
                    message.text = "Failed"
                }
            }
        }
        TextField {
            id: password
            enabled: true
            echoMode: TextInput.Password
        }
        Button {
            text: "Authenticate"
            onClicked: {
                console.log("unlock")
                authenticator.tryUnlock()
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
