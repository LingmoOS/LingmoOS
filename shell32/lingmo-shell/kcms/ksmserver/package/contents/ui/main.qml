/*
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.10
import QtQuick.Controls 2.10
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Dialogs 1.3
import org.kde.desktopsession.private 1.0 
import org.kde.kcm 1.6 as KCM

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Kirigami.InlineMessage {
            id: manualSessionRestoreRebootMessage
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Information
            visible: false
            text: i18n("The system must be restarted before manual session saving becomes active.")
            showCloseButton: true
            actions: [
                Kirigami.Action {
                    icon.name: "system-reboot"
                    text: i18n("Restart")
                    onTriggered: kcm.reboot();
                }
            ]
            Connections {
                target: kcm
                function onKsmserverSettingsChanged() {
                    if (loginManual.checked) {
                        manualSessionRestoreRebootMessage.visible = true;
                    } else {
                        manualSessionRestoreRebootMessage.visible = false;
                    }
                }
            }
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: kcm.error.length > 0 ? Kirigami.MessageType.Error : Kirigami.MessageType.Information
            visible: kcm.restartInSetupScreen || kcm.error.length > 0
            text: kcm.error.length > 0
            ? i18n("Failed to request restart to firmware setup: %1", kcm.error)
            : kcm.isUefi ? i18n("Next time the computer is restarted, it will enter the UEFI setup screen.")
            : i18n("Next time the computer is restarted, it will enter the firmware setup screen.")
            showCloseButton: true
            actions: Kirigami.Action {
                icon.name: "system-reboot"
                onTriggered: kcm.reboot();
                text: i18n("Restart Now")
            }
        }
        Kirigami.FormLayout {

            Kirigami.Separator {
                Layout.fillWidth: true
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18nc("@title:group", "Logout Screen")
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Show:")
                Kirigami.FormData.buddyFor: logoutScreenCheckbox
                spacing: 0

                CheckBox {
                    id: logoutScreenCheckbox
                    checked: Settings.confirmLogout
                    onToggled: Settings.confirmLogout = checked
                    KCM.SettingStateBinding {
                        configObject: Settings
                        settingName: "confirmLogout"
                    }
                }
                KCM.ContextualHelpButton {
                    toolTipText: xi18nc("@info", "When this setting is turned on, the logout confirmation screen will be shown when you log out, shut down, restart, press the power button, or click on buttons or menu items labeled <interface>Leave…</interface>.")
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18nc("@title:group", "Session Restore")
            }

            ButtonGroup {
                buttons: [loginRestore, loginManual, loginEmpty]
            }
            RadioButton {
                id: loginRestore
                Kirigami.FormData.label: i18n("On login, launch apps that were open:")
                text: i18nc("@option:radio Automatic style of session restoration", "On last logout")
                checked: Settings.loginMode === 0
                onToggled: Settings.loginMode = 0
                KCM.SettingStateBinding {
                    configObject: Settings
                    settingName: "loginMode"
                }
            }
            RowLayout {
                spacing: 0

                RadioButton {
                    id: loginManual
                    text: i18nc("@option:radio Manual style of session restoration", "When session was manually saved")
                    checked: Settings.loginMode === 1
                    onToggled: Settings.loginMode = 1
                    KCM.SettingStateBinding {
                        configObject: Settings
                        settingName: "loginMode"
                    }
                }
                KCM.ContextualHelpButton {
                    toolTipText: xi18nc("@info", "A <interface>Save Session</interface> button will appear in the <interface>Application Launcher</interface> menu. When you click it, Plasma will remember the apps that are open and restore them on the next login. Click it again to replace the set of remembered apps.")
                }
            }
            RadioButton {
                id: loginEmpty
                text: i18nc("@option:radio Here 'session' refers to the technical concept of session restoration, whereby the windows that were open on logout are re-opened on the next login", "Start with an empty session")
                checked: Settings.loginMode === 2
                onToggled: Settings.loginMode = 2
                KCM.SettingStateBinding {
                    configObject: Settings
                    settingName: "loginMode"
                }
            }

            Item {
                Kirigami.FormData.isSection: true
            }

            ColumnLayout {
                Kirigami.FormData.label: i18n("Ignored applications:")
                Kirigami.FormData.buddyFor: ignoredAppsTextField
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    spacing: 0

                    TextField {
                        id: ignoredAppsTextField
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 16
                        text: Settings.excludeApps
                        enabled: !loginEmpty.checked
                        // onTextEdited instead of onAccepted because otherwise the apply and
                        // reset buttons won't work, since otherwise in many case no change will
                        // be sent to the kconfigXt backend.
                        onTextEdited: Settings.excludeApps = text

                        KCM.SettingStateBinding {
                            configObject: Settings
                            settingName: "excludeApps"
                        }
                    }
                    KCM.ContextualHelpButton {
                        toolTipText: i18n("Write apps' executable names here (separated by commas or colons, for example 'xterm:konsole' or 'xterm,konsole') to prevent them from autostarting along with other session-restored apps.")
                    }
                }
            }

            Kirigami.Separator {
                Layout.fillWidth: true
                Kirigami.FormData.isSection: true
                Kirigami.FormData.label: i18nc("@title:group", "Firmware")
                visible: uefi.visible
            }

            CheckBox {
                id: uefi
                Kirigami.FormData.label: i18nc("@label:check part of a sentence: After next restart enter UEFI/Firmware setup screen", "After next restart:")
                text: kcm.isUefi ? i18nc("@option:check", "Enter UEFI setup screen")
                                : i18nc("@option:check", "Enter firmware setup screen")
                visible: kcm.canFirmwareSetup
                checked: kcm.restartInSetupScreen
                onToggled: kcm.restartInSetupScreen = checked
            }
        }
    }
}
