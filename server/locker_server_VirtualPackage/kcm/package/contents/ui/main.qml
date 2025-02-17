/*
SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kcm 1.6 as KCM
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 45
    implicitWidth: Kirigami.Units.gridUnit * 45

    ColumnLayout {
        spacing: 0

        Kirigami.FormLayout {
            RowLayout {
                Kirigami.FormData.label: i18n("Lock screen automatically:")
                QQC2.CheckBox {
                    text: i18nc("First part of sentence \"Automatically after X minutes\"", "After")
                    checked: kcm.settings.autolock
                    onToggled: kcm.settings.autolock = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "Autolock"
                    }
                }

                QQC2.SpinBox {
                    from: 1
                    editable: true
                    textFromValue: value => i18np("%1 minute", "%1 minutes", value)
                    valueFromText: text => parseInt(text)
                    value: kcm.settings.timeout
                    onValueModified: kcm.settings.timeout = value

                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "Timeout"
                    }
                }
            }

            QQC2.CheckBox {
                text: i18nc("@option:check", "After waking from sleep")
                checked: kcm.settings.lockOnResume
                onToggled: kcm.settings.lockOnResume = checked

                KCM.SettingStateBinding {
                    configObject: kcm.settings
                    settingName: "LockOnResume"
                }
            }

            Item {
                Kirigami.FormData.isSection: true
            }

            QQC2.SpinBox {
                Kirigami.FormData.label: i18nc("@label:spinbox", "Allow unlocking without password for:")
                from: 0
                to: 300
                editable: true
                textFromValue: value => i18np("%1 second", "%1 seconds", value)
                valueFromText: text => parseInt(text)
                value: kcm.settings.lockGrace
                onValueModified: kcm.settings.lockGrace = value

                KCM.SettingStateBinding {
                    configObject: kcm.settings
                    settingName: "LockGrace"
                }
            }

            Kirigami.Separator {
                Kirigami.FormData.isSection: true
            }

            KQuickControls.KeySequenceItem {
                Kirigami.FormData.label: i18n("Keyboard shortcut:")
                keySequence: kcm.settings.shortcut
                onCaptureFinished: kcm.settings.shortcut = keySequence

                KCM.SettingStateBinding {
                    configObject: kcm.settings
                    settingName: "shortcut"
                }
            }

            Item {
                Kirigami.FormData.isSection: true
            }

            QQC2.Button {
                Kirigami.FormData.label: i18n("Appearance:")
                text: i18nc("@action:button", "Configure...")
                icon.name: "preferences-desktop-theme"
                onClicked: kcm.push("Appearance.qml")

                KCM.SettingHighlighter {
                    highlight: !kcm.isDefaultsAppearance
                }
            }
        }
    }
}
