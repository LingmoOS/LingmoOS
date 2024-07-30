/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Dialogs 6.3
import QtQuick.Layouts 1.15

import org.kde.kcmutils as KCM
import org.kde.kirigami 2.14 as Kirigami
import org.kde.kitemmodels 1.0 as ItemModels
import org.kde.private.kcms.sddm 1.0

Kirigami.Page {
    title: i18nc("@title", "Behavior")

    Kirigami.FormLayout {
        width: parent.width

        RowLayout {
            Kirigami.FormData.label: i18nc("option:check", "Automatically log in:")
            spacing: Kirigami.Units.smallSpacing

            QQC2.CheckBox {
                id: autologinBox
                text: i18nc("@label:listbox, the following combobox selects the user to log in automatically", "as user:")
                checked: kcm.sddmSettings.user != ""
                KCM.SettingHighlighter {
                    highlight: (kcm.sddmSettings.user != "" && kcm.sddmSettings.defaultUser == "") ||
                                (kcm.sddmSettings.user == "" && kcm.sddmSettings.defaultUser != "")
                }
                onToggled: {
                    // Deliberately imperative because we only want the message
                    // to appear when the user checks the checkbox, not all the
                    // time when the checkbox is checked.
                    if (checked && kcm.KDEWalletAvailable()) {
                        autologinMessage.visible = true;
                    }
                }
            }
            QQC2.ComboBox {
                id: autologinUser
                model: ItemModels.KSortFilterProxyModel {
                    sourceModel: UsersModel {
                        id: userModel
                    }
                    filterRowCallback: function(sourceRow, sourceParent) {
                        const id = userModel.data(userModel.index(sourceRow, 0, sourceParent), UsersModel.UidRole)
                        return kcm.sddmSettings.minimumUid <= id && id <= kcm.sddmSettings.maximumUid
                    }
                }
                textRole: "display"
                editable: true
                onActivated: kcm.sddmSettings.user = currentText
                onEnabledChanged:  enabled ? kcm.sddmSettings.user = currentText : kcm.sddmSettings.user = ""
                KCM.SettingStateBinding {
                    visible: autologinBox.checked
                    configObject: kcm.sddmSettings
                    settingName: "User"
                    extraEnabledConditions: autologinBox.checked
                }
                Component.onCompleted: {
                    updateSelectedUser();

                    // In the initial state, comboBox sets currentIndex to 0 (the first value from the comboBox).
                    // After component is completed currentIndex changes to the correct value using `updateSelectUser` here.
                    // This implicit initial changing of the currentIndex (to 0) calls the onEditTextChanged handler,
                    // which in turn saves the wrong login in kcm.sddmSettings.user (the first value from the comboBox).
                    // So we need connect to editTextChanged signal here after the correct currentIndex was settled
                    // thus reacting only to user input and pressing the Reset/Default buttons.
                    autologinUser.editTextChanged.connect(setUserFromEditText);
                }
                function setUserFromEditText() {
                    kcm.sddmSettings.user = editText;
                }
                function updateSelectedUser() {
                    const index = find(kcm.sddmSettings.user);
                    if (index != -1) {
                        currentIndex = index;
                    }
                    editText = kcm.sddmSettings.user;
                }
                Connections { // Needed for "Reset" and "Default" buttons to work
                    target: kcm.sddmSettings
                    function onUserChanged() { autologinUser.updateSelectedUser(); }
                }
            }
            QQC2.Label {
                enabled: autologinBox.checked
                text: i18nc("@label:listbox, the following combobox selects the session that is started automatically", "with session")
            }
            QQC2.ComboBox {
                id: autologinSession
                model: SessionModel {}
                textRole: "name"
                valueRole: "file"
                onActivated: kcm.sddmSettings.session = currentValue
                onEnabledChanged: enabled ? kcm.sddmSettings.session = currentValue : kcm.sddmSettings.session = ""
                KCM.SettingStateBinding {
                    visible: autologinBox.checked
                    configObject: kcm.sddmSettings
                    settingName: "Session"
                    extraEnabledConditions: autologinBox.checked
                }
                Component.onCompleted: updateCurrentIndex()
                function updateCurrentIndex() {
                    currentIndex = Math.max(indexOfValue(kcm.sddmSettings.session), 0);
                }
                Connections { // Needed for "Reset" and "Default" buttons to work
                    target: kcm.sddmSettings
                    function onSessionChanged() { autologinSession.updateCurrentIndex(); }
                }
            }
        }
        Kirigami.InlineMessage {
            id: autologinMessage

            Layout.fillWidth: true

            type: Kirigami.MessageType.Warning

            text: xi18nc("@info", "Auto-login does not support unlocking your KDE Wallet automatically, so it will ask you to unlock it every time you log in.\
<nl/><nl/>\
To avoid this, you can change the wallet to have a blank password. Note that this is insecure and should only be done in a trusted environment.")

            actions: Kirigami.Action {
                text: i18n("Open KDE Wallet Settings")
                icon.name: "kwalletmanager"
                onTriggered: kcm.openKDEWallet();
            }
        }
        QQC2.CheckBox {
            text: i18nc("@option:check", "Log in again immediately after logging off")
            checked: kcm.sddmSettings.relogin
            onToggled: kcm.sddmSettings.relogin = checked
            KCM.SettingStateBinding {
                configObject: kcm.sddmSettings
                settingName: "Relogin"
                extraEnabledConditions: autologinBox.checked
            }
        }
        Item {
            Kirigami.FormData.isSection: true
        }
        QQC2.SpinBox {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Minimum user UID:")
            id: minSpinBox
            from: 1000
            to: maxSpinBox.value
            value: kcm.sddmSettings.minimumUid
            onValueModified: kcm.sddmSettings.minimumUid = value
            KCM.SettingStateBinding {
                configObject: kcm.sddmSettings
                settingName: "MinimumUid"
            }
        }
        QQC2.SpinBox {
            Kirigami.FormData.label: i18nc("@label:spinbox", "Maximum user UID:")
            id: maxSpinBox
            from: minSpinBox.value
            to: 60513
            value: kcm.sddmSettings.maximumUid
            onValueModified: kcm.sddmSettings.maximumUid = value
            KCM.SettingStateBinding {
                configObject: kcm.sddmSettings
                settingName: "MaximumUid"
            }
        }
        Item {
            Kirigami.FormData.isSection: true
        }
        RowLayout {
            Kirigami.FormData.label: i18nc("@label:textbox", "Halt Command:")
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.ActionTextField {
                id: haltField
                Layout.fillWidth: true
                text: kcm.sddmSettings.haltCommand
                readOnly: false
                onTextChanged: kcm.sddmSettings.haltCommand = text
                rightActions: [ Kirigami.Action {
                    icon.name: haltField.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                    visible: haltField.text.length > 0
                    onTriggered: kcm.sddmSettings.haltCommand = ""
                }]
                KCM.SettingStateBinding {
                    configObject: kcm.sddmSettings
                    settingName: "HaltCommand"
                }
            }
            QQC2.Button {
                id: haltButton
                icon.name: "document-open-folder"
                enabled: haltField.enabled
                function selectFile() {
                    fileDialog.handler = (url => kcm.sddmSettings.haltCommand = kcm.toLocalFile(url))
                    fileDialog.open()
                }
                onClicked: selectFile()
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Kirigami.FormData.label: i18nc("@label:textbox", "Reboot Command:")
            spacing: Kirigami.Units.smallSpacing

            Kirigami.ActionTextField {
                id: rebootField
                Layout.fillWidth: true
                text: kcm.sddmSettings.rebootCommand
                readOnly: false
                onTextChanged: kcm.sddmSettings.rebootCommand = text
                rightActions: [ Kirigami.Action {
                    icon.name: rebootField.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                    visible: rebootField.text.length > 0
                    onTriggered: kcm.sddmSettings.rebootCommand = ""
                }]
                KCM.SettingStateBinding {
                    configObject: kcm.sddmSettings
                    settingName: "RebootCommand"
                }
            }
            QQC2.Button {
                id: rebootButton
                icon.name: "document-open-folder"
                enabled: rebootField.enabled
                function selectFile() {
                    fileDialog.handler = (url => kcm.sddmSettings.rebootCommand = kcm.toLocalFile(url))
                    fileDialog.open()
                }
                onClicked: selectFile()
            }
        }
        FileDialog {
            id: fileDialog
            property var handler
            onAccepted: {
                handler(fileUrl)
            }
        }
    }
}
