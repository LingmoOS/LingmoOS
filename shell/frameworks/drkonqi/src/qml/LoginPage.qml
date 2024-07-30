// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.lingmoui 2.12 as LingmoUI
import org.kde.syntaxhighlighting 1.0

import org.kde.drkonqi 1.0

LingmoUI.ScrollablePage {
    id: page
    title: i18nc("@title", "Login into the bug tracking system")

    property bool loggedIn: false

    onLoggedInChanged: {
        console.log("logged in changed")
        if (loggedIn) {
            pageStack.push('qrc:/ui/ReportPage.qml')
        }
    }

    Connections {
        target: bugzilla
        function onLoginFinished(loggedIn) {
            console.log("logged in " + loggedIn)
            page.loggedIn = loggedIn
            if (rememberBox.checked) {
                credentialStore.email = emailField.text
                credentialStore.password = passwordField.text
                credentialStore.store()
            } else {
                credentialStore.drop()
            }
        }
        function onLoginError(error) {
            console.log("error " + error)
            inlineMessage.text = error
            page.enabled = true
        }
        enabled: !page.loggedIn
    }

    ColumnLayout {
        CredentialStore {
            id: credentialStore
            onEmailChanged: emailField.text = email
            onPasswordChanged: passwordField.text = password
            window: root
            Component.onCompleted: load()
        }

       LingmoUI.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            type: LingmoUI.MessageType.Error
            visible: text !== ""
        }

        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: i18nc("@info:status '1' is replaced with the short URL of the bugzilla ",
                        "You need to login with your %1 account in order to proceed.", Globals.bugzillaShortUrl);
        }
        LingmoUI.FormLayout {
            QQC2.TextField {
                id: emailField
                LingmoUI.FormData.label: i18nc("@label:textbox bugzilla account email", "E-mail Address:")
                Accessible.name: LingmoUI.FormData.label
                onAccepted: loginAction.trigger()
            }
            LingmoUI.PasswordField {
                id: passwordField
                LingmoUI.FormData.label: i18nc("@label:textbox bugzilla account password", "Password:")
                Accessible.description: LingmoUI.FormData.label
                onAccepted: loginAction.trigger()
            }
            QQC2.CheckBox {
                id: rememberBox
                checked: true
                text: i18nc("@option:check", "Save login information using the KDE Wallet system")
            }
        }
        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: xi18nc("@info/rich",
`<note>You need a user account on the <link url='%1'>KDE bug tracking system</link> in order to file a bug report, because we may need to contact you later
for requesting further information. If you do not have one, you can freely <link url='%2'>create one here</link>. Please do not use disposable email accounts.</note>`,
                                    CrashedApplication.bugReportAddress,
                                    Globals.bugzillaCreateAccountUrl)
            onLinkActivated: link => Qt.openUrlExternally(link)
        }
    }

    footer: FooterActionBar {
        actions: [
            LingmoUI.Action {
                id: loginAction
                enabled: emailField.text.length > 0 && passwordField.text.length > 0
                icon.name: "network-connect"
                text: i18nc("@action:button", "Login")
                tooltip: xi18nc("@info:tooltip", "Use this button to login to the KDE bug tracking system using the provided e-mail address and password.")
                onTriggered: {
                    bugzilla.tryLogin(emailField.text, passwordField.text)
                    page.enabled = false
                }
                Component.onCompleted: { // auto-login if possible
                    if (emailField.text !== "" && passwordField.text !== "") {
                        trigger()
                    }
                }
            }
        ]
    }

    Component.onCompleted: emailField.forceActiveFocus()
}
