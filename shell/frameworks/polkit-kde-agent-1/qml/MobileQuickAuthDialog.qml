/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.polkitkde 1.0

MobileDialogWindow {
    id: root
    title: i18n("Authentication Required")
    contentWidth: Kirigami.Units.gridUnit * 22

    property alias password: passwordField.text
    property alias inlineMessageType: inlineMessage.type
    property alias inlineMessageText: inlineMessage.text
    property alias inlineMessageIcon: inlineMessage.icon
    property alias identitiesModel: identitiesCombo.model
    property alias identitiesCurrentIndex: identitiesCombo.currentIndex
    property alias selectedIdentity: identitiesCombo.currentValue

    // passed in by QuickAuthDialog.cpp
    property string mainText
    property string subtitle
    property string descriptionString
    property string descriptionActionId
    property string descriptionVendorName
    property string descriptionVendorUrl

    signal accept()
    signal reject()

    onAccept: {
        // disable password field while password is being checked
        if (passwordField.text.length > 0) {
            passwordField.enabled = false;
        }
    }

    function authenticationFailure() {
        inlineMessage.type = Kirigami.MessageType.Error;
        inlineMessage.text = i18n("Authentication failure, please try again.");
        passwordField.clear()
        passwordField.enabled = true
        passwordField.focus = true
        rejectPasswordAnim.start();
    }

    onActiveChanged: {
        if (active) {
            // immediately focus on password field when window is focused
            passwordField.forceActiveFocus();
        }
    }

    onVisibleChanged: {
        if (visible) {
            // immediately focus on password field on load
            passwordField.forceActiveFocus();
        } else {
            // reject on close
            root.reject();
        }
    }

    // select user combobox, we are displaying its popup
    property QQC2.ComboBox selectIdentityCombobox: QQC2.ComboBox {
        id: identitiesCombo
        visible: false
        textRole: "display"
        valueRole: "userRole"
        enabled: count > 0
        model: IdentitiesModel {}
    }

    RejectPasswordAnimation {
        id: rejectPasswordAnim
        target: root.contentItem
    }

    // dialog content
    contents: ColumnLayout {
        id: column
        spacing: 0

        Kirigami.Heading {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            level: 3
            text: i18n("Authentication Required")
            wrapMode: Text.Wrap
            font.weight: Font.Bold
            horizontalAlignment: Text.AlignHCenter
        }

        QQC2.Label {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            text: root.mainText
            visible: text.length > 0
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
        }

        Kirigami.Icon {
            Layout.bottomMargin: Kirigami.Units.gridUnit
            Layout.alignment: Qt.AlignCenter
            source: "dialog-password"
            implicitWidth: Kirigami.Units.iconSizes.large
            implicitHeight: Kirigami.Units.iconSizes.large
        }

        // user information, only shown if there is more than one user to select from
        RowLayout {
            visible: identitiesCombo.count > 1
            spacing: Kirigami.Units.smallSpacing
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: Kirigami.Units.largeSpacing

            QQC2.Label {
                color: Kirigami.Theme.disabledTextColor
                text: i18n("User: %1", identitiesCombo.currentText)
                wrapMode: Text.Wrap
            }
            Kirigami.LinkButton {
                id: switchButton
                text: i18n("Switch…")
                onClicked: {
                    identitiesCombo.popup.parent = switchButton;
                    identitiesCombo.popup.open();
                }
            }
        }

        Kirigami.InlineMessage {
            id: inlineMessage
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            showCloseButton: true
            visible: text.length !== 0
        }

        Kirigami.PasswordField {
            id: passwordField
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            onAccepted: root.accept()
            placeholderText: i18n("Password…")
        }

        RowLayout {
            Layout.fillWidth: true

            QQC2.Button {
                id: details
                text: i18n("Details")
                icon.name: "documentinfo"
                checkable: true
                checked: false
            }

            Item { Layout.fillWidth: true }

            // TODO: I'm not using a DialogButtonBox right now since there is weird extra padding right on the component, is there a way to fix that?
            QQC2.Button {
                text: i18n("OK")
                icon.name: "dialog-ok"
                onClicked: root.accept()
            }

            QQC2.Button {
                text: i18n("Cancel")
                icon.name: "dialog-cancel"
                onClicked: root.reject()
            }
        }

        Kirigami.FormLayout {
            visible: details.checked
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            QQC2.Label {
                Kirigami.FormData.label: i18n("Action:")
                text: descriptionString
            }
            QQC2.Label {
                Kirigami.FormData.label: i18n("ID:")
                text: descriptionActionId
            }
            Kirigami.UrlButton {
                Kirigami.FormData.label: i18n("Vendor:")
                text: descriptionVendorName
                url: descriptionVendorUrl
            }
        }
    }
}
