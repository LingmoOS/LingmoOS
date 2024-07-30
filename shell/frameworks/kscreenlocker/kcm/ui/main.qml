/*
SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
SPDX-FileCopyrightText: 2024 Kristen McWilliam <kmcwilliampublic@gmail.com>
SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kcmutils as KCM
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 45
    implicitWidth: Kirigami.Units.gridUnit * 45

    actions:  [
        Kirigami.Action {
            text: i18nc("@action:button", "Configure Appearance…")
            icon.name: "edit-image-symbolic"
            onTriggered: kcm.push("Appearance.qml")
        }
    ]
    ColumnLayout {
        spacing: 0

        Kirigami.FormLayout {
            ComboBoxWithCustomValue {
                id: timeoutComboBox
                Layout.fillWidth: true
                Kirigami.FormData.label: i18n("Lock screen automatically:")
                textRole: "text"
                valueRole: "value"
                property var customOptions: []
                model: [
                    { value: 0, text: i18nc("Screen will not lock automatically", "Never") },
                    { value: 1, text: i18n("1 minute") },
                    { value: 2, text: i18n("2 minutes") },
                    { value: 5, text: i18n("5 minutes") },
                    { value: 10, text: i18n("10 minutes") },
                    { value: 15, text: i18n("15 minutes") },
                    { value: 30, text: i18n("30 minutes") },
                    { value: -1, text: i18nc("@option:combobox Choose a custom value outside the list of preset values", "Custom…") },
                    ...customOptions
                ]
                customRequesterValue: -1
                configuredValue: kcm.settings.timeout

                onRegularValueActivated: {
                    kcm.settings.autolock = currentValue !== 0;
                    kcm.settings.timeout = currentValue;
                }
                onCustomRequest: {
                    // Pass the current value to the dialog so it can be pre-filled in the input field.
                    customDurationPromptDialogLoader.load(customTimeoutPromptDialogComponent);
                    customDurationPromptDialogLoader.item.value = kcm.settings.timeout;
                    customDurationPromptDialogLoader.item.open();
                }
                onConfiguredValueOptionMissing: {
                    customOptions = [{
                        text: i18np("%1 minute", "%1 minutes", configuredValue),
                        value: configuredValue,
                    }];
                }

                /// Load the dialog on demand into the corresponding Loader further down.
                Component {
                    id: customTimeoutPromptDialogComponent

                    DurationPromptDialog {
                        title: i18nc("@title:window", "Custom Duration")
                        label: timeoutComboBox.Kirigami.FormData.label
                        parent: QQC2.Overlay.overlay
                        from: 1

                        acceptsUnits: [DurationPromptDialog.Unit.Minutes]

                        onAccepted: {
                            kcm.settings.autolock = value !== 0;
                            kcm.settings.timeout = value;
                            timeoutComboBox.customRequestCompleted();
                        }
                        onRejected: {
                            timeoutComboBox.customRequestCompleted();
                        }
                    }
                }

                KCM.SettingStateBinding {
                    configObject: kcm.settings
                    settingName: "Timeout"
                }
            }

            QQC2.CheckBox {
                text: i18nc("@option:check", "Lock after waking from sleep")
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

            ComboBoxWithCustomValue {
                id: lockGraceComboBox
                Layout.fillWidth: true
                Kirigami.FormData.label: i18nc("First part of sentence \"Delay before password required: X minutes\"", "Delay before password required:")
                textRole: "text"
                valueRole: "seconds"
                property var customOptions: []
                model: [
                    { seconds: 0, text: i18nc("The grace period is disabled", "Require password immediately"), unit: DurationPromptDialog.Unit.Seconds },
                    { seconds: -1, text: i18nc("Password not required", "Never require password"), skipPassword: true },
                    { seconds: 5, text: i18n("5 seconds"), unit: DurationPromptDialog.Unit.Seconds },
                    { seconds: 30, text: i18n("30 seconds"), unit: DurationPromptDialog.Unit.Seconds },
                    { seconds: 1 * 60, text: i18n("1 minute"), unit: DurationPromptDialog.Unit.Minutes },
                    { seconds: 5 * 60, text: i18n("5 minutes"), unit: DurationPromptDialog.Unit.Minutes },
                    { seconds: 15 * 60, text: i18n("15 minutes"), unit: DurationPromptDialog.Unit.Minutes },
                    { seconds: -2, text: i18nc("@option:combobox Choose a custom value outside the list of preset values", "Custom…") },
                    ...customOptions
                ]
                customRequesterValue: -2
                configuredValue: kcm.settings.requirePassword ? kcm.settings.lockGrace : -1

                onRegularValueActivated: {
                    if (model[currentIndex].skipPassword === true) {
                        kcm.settings.requirePassword = false;
                        return;
                    } else {
                        kcm.settings.lockGrace = currentValue;
                        kcm.settings.requirePassword = true;
                    }
                }
                onCustomRequest: {
                    // Pass the current value to the dialog so it can be pre-filled in the input field.
                    const currentOptionIndex = indexOfValue(kcm.settings.lockGrace);
                    const currentOption = currentOptionIndex !== -1
                        ? model[currentOptionIndex]
                        : { seconds: kcm.settings.lockGrace, unit: DurationPromptDialog.Unit.Seconds };

                    customDurationPromptDialogLoader.load(customLockGracePromptDialogComponent);
                    customDurationPromptDialogLoader.item.unit = currentOption.unit;
                    customDurationPromptDialogLoader.item.value =
                        currentOption.unit === DurationPromptDialog.Unit.Minutes
                            ? (currentOption.seconds / 60)
                            : currentOption.seconds;
                    customDurationPromptDialogLoader.item.open();
                }
                onConfiguredValueOptionMissing: {
                    const isMinutes = configuredValue % 60 === 0 &&
                    (customUnit ?? DurationPromptDialog.Unit.Minutes) !== DurationPromptDialog.Unit.Seconds;
                    customOptions = [{
                        seconds: configuredValue,
                        text: isMinutes
                            ? i18np("%1 minute", "%1 minutes", configuredValue / 60)
                            : i18np("%1 second", "%1 seconds", configuredValue),
                        unit: isMinutes ? DurationPromptDialog.Unit.Minutes : DurationPromptDialog.Unit.Seconds,
                    }];
                    customUnit = null;
                }
                property var customUnit: null

                /// Component prevents the dialog from being loaded until the loader loads it.
                Component {
                    id: customLockGracePromptDialogComponent

                    DurationPromptDialog {
                        title: i18nc("@title:window", "Custom Duration")
                        label: lockGraceComboBox.Kirigami.FormData.label
                        parent: QQC2.Overlay.overlay
                        from: 1

                        acceptsUnits: [DurationPromptDialog.Unit.Seconds, DurationPromptDialog.Unit.Minutes]

                        onAccepted: function() {
                            // Set the combo box's customUnit prior to configuredValue,
                            // so the selected unit is set explicitly instead of guessed by modulo.
                            lockGraceComboBox.customUnit = unit;
                            kcm.settings.lockGrace =
                                unit === DurationPromptDialog.Unit.Minutes ? (value * 60) : value;
                            kcm.settings.requirePassword = true;
                            lockGraceComboBox.customRequestCompleted();
                        }

                        onRejected: function() {
                            lockGraceComboBox.customRequestCompleted();
                        }
                    }
                }

                KCM.SettingStateBinding {
                    extraEnabledConditions: kcm.settings.autolock
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
        }
    }

    /// Dialog handled by a Loader to avoid loading it until it is needed.
    Loader {
        id: customDurationPromptDialogLoader
        anchors.centerIn: parent

        /// Load the dialog if it is not already loaded, or change it for a different one.
        function load(component) {
            if (sourceComponent !== component) {
                sourceComponent = component;
            }
        }
    }
}
