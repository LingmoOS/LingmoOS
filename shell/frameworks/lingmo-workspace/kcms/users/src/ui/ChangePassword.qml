/*
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.3

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.kcm.users 1.0 as UsersKCM

LingmoUI.PromptDialog {
    id: passwordRoot

    preferredWidth: LingmoUI.Units.gridUnit * 20

    function openAndClear() {
        verifyField.text = ""
        passwordField.text = ""
        passwordField.forceActiveFocus()
        open()
    }

    property UsersKCM.User user

    title: i18n("Change Password")

    standardButtons: LingmoUI.Dialog.Cancel
    customFooterActions: LingmoUI.Action {
        id: passAction
        text: i18n("Set Password")
        enabled: !passwordWarning.visible && verifyField.text && passwordField.text
        onTriggered: {
            if (passwordField.text !== verifyField.text) {
                debouncer.isTriggered = true
                return
            }
            passwordRoot.user.setPassword(passwordField.text)
            passwordRoot.close()
        }
    }

    ColumnLayout {
        id: mainColumn
        spacing: LingmoUI.Units.smallSpacing

        LingmoUI.PasswordField {
            id: passwordField

            Layout.fillWidth: true

            placeholderText: i18n("Password")
            onTextChanged: debouncer.reset()

            onAccepted: {
                if (!passwordWarning.visible && verifyField.text && passwordField.text) {
                    passAction.trigger()
                }
            }
        }

        LingmoUI.PasswordField {
            id: verifyField

            Layout.fillWidth: true

            placeholderText: i18n("Confirm password")
            onTextChanged: debouncer.reset()

            onAccepted: {
                if (!passwordWarning.visible && verifyField.text && passwordField.text) {
                    passAction.trigger()
                }
            }
        }

        Debouncer {
            id: debouncer
        }

        LingmoUI.InlineMessage {
            id: passwordWarning

            Layout.fillWidth: true
            type: LingmoUI.MessageType.Error
            text: i18n("Passwords must match")
            visible: passwordField.text !== ""
                && verifyField.text !== ""
                && passwordField.text !== verifyField.text
                && debouncer.isTriggered
        }
    }
}
