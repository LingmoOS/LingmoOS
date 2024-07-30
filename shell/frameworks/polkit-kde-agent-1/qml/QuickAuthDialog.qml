/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023 Devin Lin <devin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui 2.19 as LingmoUI
import org.kde.polkitkde 1.0

LingmoUI.AbstractApplicationWindow {
    id: root
    title: i18n("Authentication Required")

    maximumHeight: intendedWindowHeight
    minimumHeight: intendedWindowHeight
    minimumWidth: intendedWindowWidth
    maximumWidth: intendedWindowWidth
    width: intendedWindowWidth
    height: intendedWindowHeight

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
        if (passwordField.text !== "") {
            passwordField.enabled = false;
        }
    }

    Shortcut {
        sequence: StandardKey.Cancel
        onActivated: root.reject()
    }

    function authenticationFailure() {
        inlineMessage.type = LingmoUI.MessageType.Error;
        inlineMessage.text = i18n("Authentication failure, please try again.");
        passwordField.clear()
        passwordField.enabled = true
        passwordField.focus = true
        rejectPasswordAnim.start();
    }

    readonly property real intendedWindowWidth: LingmoUI.Units.gridUnit * 35
    // HACK: Window doesn't seem to automatically update height from binding, so manually set the height value
    readonly property real intendedWindowHeight: column.implicitHeight + column.anchors.topMargin + column.anchors.bottomMargin;
    onIntendedWindowHeightChanged: {
        minimumHeight = intendedWindowHeight;
        height = intendedWindowHeight;
        maximumHeight = intendedWindowHeight
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
        model: IdentitiesModel {
            id: identitiesModel
        }
    }

    // add separator to window decorations
    LingmoUI.Separator {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    RejectPasswordAnimation {
        id: rejectPasswordAnim
        target: column
    }

    // dialog contents
    ColumnLayout {
        id: column
        spacing: 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: LingmoUI.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: LingmoUI.Units.largeSpacing

            LingmoUI.Icon {
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                Layout.rightMargin: LingmoUI.Units.gridUnit
                Layout.topMargin: LingmoUI.Units.largeSpacing
                Layout.alignment: Qt.AlignTop
                source: "dialog-password"
                implicitWidth: LingmoUI.Units.iconSizes.huge
                implicitHeight: LingmoUI.Units.iconSizes.huge
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: LingmoUI.Units.largeSpacing

                LingmoUI.Heading {
                    Layout.fillWidth: true
                    level: 3
                    text: root.mainText
                    wrapMode: Text.Wrap
                    font.weight: Font.Bold
                }

                // user information, only shown if there is more than one user to select from
                Flow {
                    spacing: LingmoUI.Units.smallSpacing
                    Layout.fillWidth: true
                    Layout.bottomMargin: LingmoUI.Units.smallSpacing

                    QQC2.Label {
                        color: LingmoUI.Theme.disabledTextColor
                        text: i18n("Authenticating as %1", identitiesCombo.currentText)
                        wrapMode: Text.Wrap
                    }

                    LingmoUI.LinkButton {
                        id: switchButton
                        text: i18n("Switch…")
                        visible: identitiesCombo.count > 1
                        onClicked: {
                            identitiesCombo.popup.parent = switchButton;
                            identitiesCombo.popup.open();
                        }
                    }
                }

                LingmoUI.InlineMessage {
                    id: inlineMessage
                    Layout.fillWidth: true
                    showCloseButton: true
                    visible: text.length !== 0
                }

                LingmoUI.PasswordField {
                    id: passwordField
                    Layout.fillWidth: true
                    onAccepted: root.accept()
                    placeholderText: i18n("Password…")
                }

                RowLayout {
                    id: passwordRow
                    Layout.fillWidth: true

                    QQC2.Button {
                        id: details
                        text: i18n("Details")
                        icon.name: "documentinfo"
                        checkable: true
                        checked: false
                    }

                    Item {
                        Layout.fillWidth: true
                    }

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
            }
        }

        LingmoUI.FormLayout {
            visible: details.checked
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            QQC2.Label {
                LingmoUI.FormData.label: i18n("Action:")
                text: descriptionString
            }
            QQC2.Label {
                LingmoUI.FormData.label: i18n("ID:")
                text: descriptionActionId
            }
            LingmoUI.UrlButton {
                LingmoUI.FormData.label: i18n("Vendor:")
                text: descriptionVendorName
                url: descriptionVendorUrl
            }
        }
    }
}
