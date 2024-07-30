/*
 *  SPDX-FileCopyrightText: 2022, 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.powerdevil as PD

Kirigami.FormLayout {
    id: root

    required property string profileId
    required property string profileLabel
    readonly property var profileSettings: kcm.settings["profile" + profileId]

    function formatPercentageText(value) {
        return i18nc(
            "Percentage value example, used for formatting brightness levels in the power management settings",
            "10%"
        ).replace("10", value);
    }

    //
    // Suspend Session

    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Suspend Session")
        visible: (
            autoSuspendActionRow.visible
            || powerButtonActionCombo.visible
            || lidActionCombo.visible
            || triggersLidActionWhenExternalMonitorPresentCheck.visible
            || sleepModeCombo.visible
        )
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.isSection: true
        visible: autoSuspendActionCombo.visible && !autoSuspendActionCombo.isConfiguredValueSupported
        Layout.fillWidth: true
        type: Kirigami.MessageType.Warning
        text: i18nc("@info:status", "The action you had previously configured for after a period of inactivity is now unsupported on your system. Please select a different one.")
    }

    RowLayout {
        id: autoSuspendActionRow
        Kirigami.FormData.label: i18nc(
            "@label:combobox Suspend action such as sleep/hibernate to perform when the system is idle",
            "A&fter a period of inactivity:"
        )
        visible: kcm.supportedActions["SuspendSession"] === true
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        ComboBoxWithIcon {
            id: autoSuspendActionCombo
            Accessible.name: i18nc("@accessible:name:combobox", "Action to perform when the system is idle")

            Layout.fillWidth: true
            implicitContentWidthPolicy: QQC2.ComboBox.WidestTextWhenCompleted

            model: kcm.autoSuspendActionModel
            textRole: "name"
            valueRole: "value"

            readonly property bool isConfiguredValueSupported: currentValue === profileSettings.autoSuspendAction

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "AutoSuspendAction"
            }
            Component.onCompleted: {
                // indexOfValue() is invalid before onCompleted, so wait until here to bind currentIndex.
                currentIndex = Qt.binding(() => indexOfValue(profileSettings.autoSuspendAction));
            }
            onActivated: {
                profileSettings.autoSuspendAction = currentValue;
            }
        }

        TimeDurationComboBox {
            id: autoSuspendIdleTimeoutCombo
            durationPromptLabel: autoSuspendActionRow.Kirigami.FormData.label
            durationPromptAcceptsUnits: [DurationPromptDialog.Unit.Seconds, DurationPromptDialog.Unit.Minutes]

            function translateSeconds(n, formatUnit = DurationPromptDialog.Unit.Seconds) {
                return formatUnit == DurationPromptDialog.Unit.Minutes
                    ? translateMinutes(n / 60) : i18ncp("@option:combobox", "After %1 second", "After %1 seconds", n);
            }
            function translateMinutes(n) { return i18ncp("@option:combobox", "After %1 minute", "After %1 minutes", n); }

            valueRole: "seconds"
            textRole: "text"
            unitOfValueRole: DurationPromptDialog.Unit.Seconds
            durationPromptFromValue: 60
            presetOptions: [
                { seconds: 1 * 60, text: translateMinutes(1), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 2 * 60, text: translateMinutes(2), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 5 * 60, text: translateMinutes(5), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 10 * 60, text: translateMinutes(10), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 15 * 60, text: translateMinutes(15), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 30 * 60, text: translateMinutes(30), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: -1, text: i18nc("@option:combobox Choose a custom value outside the list of preset values", "Custom…") },
            ]
            customRequesterValue: -1
            configuredValue: profileSettings.autoSuspendIdleTimeoutSec
            configuredDisplayUnit: model[indexOfValue(configuredValue)]?.unit ?? DurationPromptDialog.Unit.Minutes

            onRegularValueActivated: { profileSettings.autoSuspendIdleTimeoutSec = currentValue; }
            onCustomDurationAccepted: {
                profileSettings.autoSuspendIdleTimeoutSec = valueToUnit(
                    customDuration.value, customDuration.unit, DurationPromptDialog.Unit.Seconds);
            }

            onConfiguredValueOptionMissing: {
                const unit = configuredValue % 60 === 0
                    ? customDuration?.unit ?? DurationPromptDialog.Unit.Minutes
                    : DurationPromptDialog.Unit.Seconds;

                customOptions = [{
                    seconds: configuredValue,
                    text: translateSeconds(configuredValue, unit),
                    unit: unit,
                }];
                customDuration = null;
            }

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "AutoSuspendIdleTimeoutSec"
                extraEnabledConditions: autoSuspendActionCombo.currentValue !== PD.PowerDevil.PowerButtonAction.NoAction
            }
        }
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.isSection: true
        visible: powerButtonActionCombo.visible && !powerButtonActionCombo.isConfiguredValueSupported
        Layout.fillWidth: true
        type: Kirigami.MessageType.Warning
        text: i18nc("@info:status", "The action you had previously configured for when the power button is pressed is now unsupported on your system. Please select a different one.")
    }

    ComboBoxWithIcon {
        id: powerButtonActionCombo
        Kirigami.FormData.label: i18nc(
            "@label:combobox Suspend action such as sleep/hibernate to perform when the power button is pressed",
            "When &power button pressed:"
        )
        Accessible.name: i18nc("@accessible:name:combobox", "Action to perform when the power button is pressed")

        visible: kcm.supportedActions["HandleButtonEvents"] === true && kcm.isPowerButtonPresent
        Layout.fillWidth: true
        implicitContentWidthPolicy: QQC2.ComboBox.WidestTextWhenCompleted

        model: kcm.powerButtonActionModel
        textRole: "name"
        valueRole: "value"

        readonly property bool isConfiguredValueSupported: currentValue === profileSettings.powerButtonAction

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "PowerButtonAction"
        }
        Component.onCompleted: {
            // indexOfValue() is invalid before onCompleted, so wait until here to bind currentIndex.
            currentIndex = Qt.binding(() => indexOfValue(profileSettings.powerButtonAction));
        }
        onActivated: {
            profileSettings.powerButtonAction = currentValue;
        }
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.isSection: true
        visible: lidActionCombo.visible && !lidActionCombo.isConfiguredValueSupported
        Layout.fillWidth: true
        type: Kirigami.MessageType.Warning
        text: i18nc("@info:status", "The action you had previously configured for when the lid is closed is now unsupported on your system. Please select a different one.")
    }

    ComboBoxWithIcon {
        id: lidActionCombo
        Kirigami.FormData.label: i18nc(
            "@label:combobox Suspend action such as sleep/hibernate to perform when the power button is pressed",
            "When laptop &lid closed:"
        )
        Accessible.name: i18nc("@accessible:name:combobox", "Action to perform when the laptop lid is closed")

        visible: kcm.supportedActions["HandleButtonEvents"] === true && kcm.isLidPresent
        Layout.fillWidth: true
        implicitContentWidthPolicy: QQC2.ComboBox.WidestTextWhenCompleted

        model: kcm.lidActionModel
        textRole: "name"
        valueRole: "value"

        readonly property bool isConfiguredValueSupported: currentValue === profileSettings.lidAction

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "LidAction"
        }
        Component.onCompleted: {
            // indexOfValue() is invalid before onCompleted, so wait until here to bind currentIndex.
            currentIndex = Qt.binding(() => indexOfValue(profileSettings.lidAction));
        }
        onActivated: {
            profileSettings.lidAction = currentValue;
        }
    }

    QQC2.CheckBox {
        id: triggersLidActionWhenExternalMonitorPresentCheck
        text: i18nc(
            "@text:checkbox Trigger laptop lid action even when an external monitor is connected",
            "Even when an external monitor is connected"
        )
        Accessible.name: i18n("Perform laptop lid action even when an external monitor is connected")

        visible: lidActionCombo.visible

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "InhibitLidActionWhenExternalMonitorPresent"
            extraEnabledConditions: lidActionCombo.currentValue !== PD.PowerDevil.PowerButtonAction.NoAction
        }
        checked: !profileSettings.inhibitLidActionWhenExternalMonitorPresent
        onToggled: { profileSettings.inhibitLidActionWhenExternalMonitorPresent = !checked; }
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.isSection: true
        visible: sleepModeCombo.visible && sleepModeCombo.enabled && !sleepModeCombo.isConfiguredValueSupported
        Layout.fillWidth: true
        type: Kirigami.MessageType.Warning
        text: i18nc("@info:status", "The sleep mode you had previously configured is now unsupported on your system. Please select a different one.")
    }

    QQC2.ComboBox {
        id: sleepModeCombo
        Kirigami.FormData.label: i18nc(
            "@label:combobox Sleep mode selection - suspend to memory, disk or both",
            "When sleeping, enter:"
        )
        Accessible.name: i18nc("@accessible:name:combobox", "When sleeping, enter this power-save mode")

        visible: count > 1 && (kcm.supportedActions["SuspendSession"] === true || kcm.supportedActions["HandleButtonEvents"] === true)
        Layout.fillWidth: true
        implicitContentWidthPolicy: QQC2.ComboBox.WidestTextWhenCompleted

        model: kcm.sleepModeModel
        textRole: "name"
        valueRole: "value"

        delegate: Kirigami.SubtitleDelegate {
            required property string index

            // model roles from roleNames(), expose these in your QAbstractItemModel if you haven't already
            required property string name
            required property string subtext

            text: name
            subtitle: subtext
            icon.width: 0
            width: sleepModeCombo.popup.width
            highlighted: index === sleepModeCombo.currentIndex
        }

        readonly property bool isConfiguredValueSupported: currentValue === profileSettings.sleepMode

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "SleepMode"
            extraEnabledConditions: (
                autoSuspendActionCombo.currentValue === PD.PowerDevil.PowerButtonAction.Sleep
                || powerButtonActionCombo.currentValue === PD.PowerDevil.PowerButtonAction.Sleep
                || lidActionCombo.currentValue === PD.PowerDevil.PowerButtonAction.Sleep
            )
        }
        Component.onCompleted: {
            // indexOfValue() is invalid before onCompleted, so wait until here to bind currentIndex.
            currentIndex = Qt.binding(() => indexOfValue(profileSettings.sleepMode));
        }
        onActivated: {
            profileSettings.sleepMode = currentValue;
        }
    }

    //
    // Display and Brightness

    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Display and Brightness")
        visible: (
            kcm.supportedActions["BrightnessControl"] === true
            || kcm.supportedActions["DimDisplay"] === true
            || kcm.supportedActions["DPMSControl"] === true
            || kcm.supportedActions["KeyboardBrightnessControl"] === true
        )
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("@label:slider Brightness level", "Change scr&een brightness:")

        visible: kcm.supportedActions["BrightnessControl"] === true
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            id: displayBrightnessCheck

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "UseProfileSpecificDisplayBrightness"
            }
            checked: profileSettings.useProfileSpecificDisplayBrightness
            onToggled: { profileSettings.useProfileSpecificDisplayBrightness = checked; }
        }
        QQC2.Slider {
            id: displayBrightnessSlider
            Layout.fillWidth: true
            from: 1
            to: 100
            stepSize: 1

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "DisplayBrightness"
                extraEnabledConditions: displayBrightnessCheck.checked
            }
            value: profileSettings.displayBrightness
            onMoved: { profileSettings.displayBrightness = value; }
        }
        QQC2.Label {
            enabled: displayBrightnessCheck.checked
            text: formatPercentageText(displayBrightnessSlider.value)
            Layout.preferredWidth: displayBrightnessPercentageMetrics.width
        }
        TextMetrics {
            id: displayBrightnessPercentageMetrics
            text: formatPercentageText(100)
        }
    }

    TimeDurationComboBox {
        id: dimDisplayIdleTimeoutCombo
        visible: kcm.supportedActions["DimDisplay"] === true
        Layout.fillWidth: true
        Kirigami.FormData.label: i18nc("@label:combobox Dim screen after X minutes", "Di&m automatically:")

        durationPromptLabel: i18nc("@label:spinbox Dim screen after X minutes", "Di&m automatically after:")
        durationPromptAcceptsUnits: [DurationPromptDialog.Unit.Seconds, DurationPromptDialog.Unit.Minutes]

        function translateSeconds(n, formatUnit = DurationPromptDialog.Unit.Seconds) {
            return formatUnit == DurationPromptDialog.Unit.Minutes
                ? translateMinutes(n / 60) : i18ncp("@option:combobox", "%1 second", "%1 seconds", n);
        }
        function translateMinutes(n) { return i18ncp("@option:combobox", "%1 minute", "%1 minutes", n); }

        valueRole: "seconds"
        textRole: "text"
        unitOfValueRole: DurationPromptDialog.Unit.Seconds
        durationPromptFromValue: 10
        presetOptions: [
            { seconds: -1, text: i18nc("@option:combobox Dim screen automatically", "Never") },
            { seconds: 30, text: translateSeconds(30), unit: DurationPromptDialog.Unit.Seconds },
            { seconds: 1 * 60, text: translateMinutes(1), unit: DurationPromptDialog.Unit.Minutes },
            { seconds: 2 * 60, text: translateMinutes(2), unit: DurationPromptDialog.Unit.Minutes },
            { seconds: 5 * 60, text: translateMinutes(5), unit: DurationPromptDialog.Unit.Minutes },
            { seconds: 10 * 60, text: translateMinutes(10), unit: DurationPromptDialog.Unit.Minutes },
            { seconds: 15 * 60, text: translateMinutes(15), unit: DurationPromptDialog.Unit.Minutes },
            { seconds: -2, text: i18nc("@option:combobox Choose a custom value outside the list of preset values", "Custom…") },
        ]
        customRequesterValue: -2
        configuredValue: profileSettings.dimDisplayWhenIdle ? profileSettings.dimDisplayIdleTimeoutSec : -1
        configuredDisplayUnit: model[indexOfValue(configuredValue)]?.unit ?? DurationPromptDialog.Unit.Minutes

        onRegularValueActivated: {
            profileSettings.dimDisplayIdleTimeoutSec = currentValue;
            profileSettings.dimDisplayWhenIdle = currentValue > 0;
        }
        onCustomDurationAccepted: {
            profileSettings.dimDisplayIdleTimeoutSec = valueToUnit(
                customDuration.value, customDuration.unit, DurationPromptDialog.Unit.Seconds);
            profileSettings.dimDisplayWhenIdle = customDuration.value > 0;
        }

        onConfiguredValueOptionMissing: {
            const unit = configuredValue % 60 === 0
                ? customDuration?.unit ?? DurationPromptDialog.Unit.Minutes
                : DurationPromptDialog.Unit.Seconds;

            customOptions = [{
                seconds: configuredValue,
                text: translateSeconds(configuredValue, unit),
                unit: unit,
            }];
            customDuration = null;
        }

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "DimDisplayIdleTimeoutSec"
        }
    }

    Kirigami.InlineMessage {
        Kirigami.FormData.isSection: true
        visible: (
            dimDisplayIdleTimeoutCombo.visible
            && profileSettings.dimDisplayWhenIdle && profileSettings.turnOffDisplayWhenIdle
            && profileSettings.dimDisplayIdleTimeoutSec >= profileSettings.turnOffDisplayIdleTimeoutSec
            && profileSettings.dimDisplayIdleTimeoutSec >= profileSettings.turnOffDisplayIdleTimeoutWhenLockedSec
        )
        Layout.fillWidth: true
        type: Kirigami.MessageType.Warning
        text: i18nc("@info:status", "The screen will not be dimmed because it is configured to turn off sooner.")
    }

    RowLayout {
        id: turnOffDisplayRow
        Kirigami.FormData.label: i18nc("@label:combobox After X minutes", "&Turn off screen:")
        Kirigami.FormData.buddyFor: turnOffDisplayIdleTimeoutCombo

        visible: kcm.supportedActions["DPMSControl"] === true
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        TimeDurationComboBox {
            id: turnOffDisplayIdleTimeoutCombo
            Layout.fillWidth: true

            durationPromptAcceptsUnits: [DurationPromptDialog.Unit.Seconds, DurationPromptDialog.Unit.Minutes]
            durationPromptLabel: i18nc("@label:spinbox After X minutes", "Turn off screen after:")

            function translateSeconds(n, formatUnit = DurationPromptDialog.Unit.Seconds) {
                return formatUnit == DurationPromptDialog.Unit.Minutes
                    ? translateMinutes(n / 60) : i18ncp("@option:combobox Turn off screen (caution: watch for string length)", "%1 second", "%1 seconds", n);
            }
            function translateMinutes(n) { return i18ncp("@option:combobox Turn off screen (caution: watch for string length)", "%1 minute", "%1 minutes", n); }

            valueRole: "seconds"
            textRole: "text"
            unitOfValueRole: DurationPromptDialog.Unit.Seconds
            durationPromptFromValue: 30
            presetOptions: [
                { seconds: -1, text: i18nc("@option:combobox Turn off screen (caution: watch for string length)", "Never") },
                { seconds: 1 * 60, text: translateMinutes(1), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 2 * 60, text: translateMinutes(2), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 5 * 60, text: translateMinutes(5), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 10 * 60, text: translateMinutes(10), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 15 * 60, text: translateMinutes(15), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 30 * 60, text: translateMinutes(30), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: -2, text: i18nc("@option:combobox Choose a custom value outside the list of preset values (caution: watch for string length)", "Custom…") },
            ]
            customRequesterValue: -2
            configuredValue: profileSettings.turnOffDisplayWhenIdle ? profileSettings.turnOffDisplayIdleTimeoutSec : -1
            configuredDisplayUnit: model[indexOfValue(configuredValue)]?.unit ?? DurationPromptDialog.Unit.Minutes

            onRegularValueActivated: {
                profileSettings.turnOffDisplayIdleTimeoutSec = currentValue;
                profileSettings.turnOffDisplayWhenIdle = currentValue > 0;
            }
            onCustomDurationAccepted: {
                profileSettings.turnOffDisplayIdleTimeoutSec = valueToUnit(
                    customDuration.value, customDuration.unit, DurationPromptDialog.Unit.Seconds);
                profileSettings.turnOffDisplayWhenIdle = customDuration.value > 0;
            }

            onConfiguredValueOptionMissing: {
                const unit = configuredValue % 60 === 0
                    ? customDuration?.unit ?? DurationPromptDialog.Unit.Minutes
                    : DurationPromptDialog.Unit.Seconds;

                customOptions = [{
                    seconds: configuredValue,
                    text: translateSeconds(configuredValue, unit),
                    unit: unit,
                }];
                customDuration = null;
            }

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "TurnOffDisplayIdleTimeoutSec"
            }
        }

        TimeDurationComboBox {
            id: turnOffDisplayIdleTimeoutWhenLockedCombo

            durationPromptAcceptsUnits: [DurationPromptDialog.Unit.Seconds, DurationPromptDialog.Unit.Minutes]
            durationPromptLabel: i18nc("@label:spinbox After X minutes", "When locked, turn off screen after:")

            function translateSeconds(n, formatUnit = DurationPromptDialog.Unit.Seconds) {
                return formatUnit == DurationPromptDialog.Unit.Minutes
                    ? translateMinutes(n / 60) : i18ncp("@option:combobox Turn off screen (caution: watch for string length)", "When locked: %1 second", "When locked: %1 seconds", n);
            }
            function translateMinutes(n) { return i18ncp("@option:combobox Turn off screen (caution: watch for string length)", "When locked: %1 minute", "When locked: %1 minutes", n); }

            valueRole: "seconds"
            textRole: "text"
            unitOfValueRole: DurationPromptDialog.Unit.Seconds
            durationPromptFromValue: 10
            presetOptions: [
                { seconds: -2, text: i18nc("@option:combobox Turn off screen after X minutes regardless of lock screen (caution: watch for string length)", "When locked and unlocked")  },
                // -1 would be "Never", same as for the unlocked timeout, if we want that option
                { seconds: 0, text: i18nc("@option:combobox Turn off screen (caution: watch for string length)", "When locked: Immediately") },
                { seconds: 20, text: translateSeconds(20), unit: DurationPromptDialog.Unit.Seconds },
                { seconds: 1 * 60, text: translateMinutes(1), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 2 * 60, text: translateMinutes(2), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 5 * 60, text: translateMinutes(5), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: -3, text: i18nc("@option:combobox Choose a custom value outside the list of preset values", "Custom…") },
            ]
            customRequesterValue: -3
            configuredValue: profileSettings.turnOffDisplayIdleTimeoutWhenLockedSec
            configuredDisplayUnit: model[indexOfValue(configuredValue)]?.unit ?? DurationPromptDialog.Unit.Seconds

            onRegularValueActivated: { profileSettings.turnOffDisplayIdleTimeoutWhenLockedSec = currentValue; }
            onCustomDurationAccepted: {
                profileSettings.turnOffDisplayIdleTimeoutWhenLockedSec = valueToUnit(
                    customDuration.value, customDuration.unit, DurationPromptDialog.Unit.Seconds);
            }

            onConfiguredValueOptionMissing: {
                const unit = configuredValue % 60 === 0
                    ? customDuration?.unit ?? DurationPromptDialog.Unit.Minutes
                    : DurationPromptDialog.Unit.Seconds;

                customOptions = [{
                    seconds: configuredValue,
                    text: translateSeconds(configuredValue, unit),
                    unit: unit,
                }];
                customDuration = null;
            }

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "TurnOffDisplayIdleTimeoutWhenLockedSec"
                extraEnabledConditions: profileSettings.turnOffDisplayWhenIdle
            }
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("@label:slider Brightness level", "Change key&board brightness:")

        visible: kcm.supportedActions["KeyboardBrightnessControl"] === true
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        QQC2.CheckBox {
            id: keyboardBrightnessCheck

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "UseProfileSpecificKeyboardBrightness"
            }
            checked: profileSettings.useProfileSpecificKeyboardBrightness
            onToggled: { profileSettings.useProfileSpecificKeyboardBrightness = checked; }
        }
        QQC2.Slider {
            id: keyboardBrightnessSlider
            Layout.fillWidth: true
            from: 0
            to: 100
            stepSize: 1

            KCM.SettingStateBinding {
                configObject: profileSettings
                settingName: "KeyboardBrightness"
                extraEnabledConditions: keyboardBrightnessCheck.checked
            }
            value: profileSettings.keyboardBrightness
            onMoved: { profileSettings.keyboardBrightness = value; }
        }
        QQC2.Label {
            enabled: keyboardBrightnessCheck.checked
            text: formatPercentageText(keyboardBrightnessSlider.value)
            Layout.preferredWidth: keyboardBrightnessPercentageMetrics.width
        }
        TextMetrics {
            id: keyboardBrightnessPercentageMetrics
            text: formatPercentageText(100)
        }
    }

    //
    // Advanced customization options

    Item {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18nc("@title:group", "Other Settings")
        visible: kcm.supportedActions["RunScript"] === true || powerProfileCombo.visible
    }

    QQC2.ComboBox {
        id: powerProfileCombo
        Kirigami.FormData.label: i18nc(
            "@label:combobox Power Save, Balanced or Performance profile - same options as in the Battery applet",
            "Switch to po&wer profile:"
        )
        Accessible.name: i18nc(
            "@accessible:name:combobox Power Save, Balanced or Performance profile - same options as in the Battery applet",
            "Switch to power profile"
        )

        visible: kcm.supportedActions["PowerProfile"] === true && count > 1
        Layout.fillWidth: true
        implicitContentWidthPolicy: QQC2.ComboBox.WidestTextWhenCompleted

        model: kcm.powerProfileModel
        textRole: "name"
        valueRole: "value"

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "PowerProfile"
        }
        Component.onCompleted: {
            // indexOfValue() is invalid before onCompleted, so wait until here to bind currentIndex.
            // Also observe count - PowerProfileModel has delayed initialization due to a D-Bus call.
            currentIndex = Qt.binding(() => count ? indexOfValue(profileSettings.powerProfile) : -1);
        }
        onActivated: {
            profileSettings.powerProfile = currentValue;
        }
    }

    QQC2.Button {
        id: addScriptCommandButton
        Kirigami.FormData.label: i18nc("@label:button", "Run custom script:")

        visible: (kcm.supportedActions["RunScript"] === true
            // If power states aren't switchable, only show the idleTimeoutCommand field
            // because profile load and unload fields are not useful, just confusing.
            && kcm.supportsBatteryProfiles
        )
        icon.name: "settings-configure"
        text: i18nc(
            "@text:button Determine what will trigger a script command to run in this power state",
            "Choose run conditions…"
        )
        Accessible.name: i18nc("@accessible:name:button", "Choose run conditions for script command")

        onClicked: {
            if (addScriptCommandMenu.opened) {
                addScriptCommandMenu.close(); // closePolicy: Popup.CloseOnPressOutside does not seem to work?
            } else {
                addScriptCommandMenu.open();
            }
        }

        QQC2.Menu {
            id: addScriptCommandMenu
            title: addScriptCommandButton.text
            y: addScriptCommandButton.height

            Kirigami.Action {
                id: profileLoadCommandEditAction
                text: i18nc(
                    "@text:action:menu Script command to execute for power state (On AC Power, On Battery, ...)",
                    "When entering \"%1\" state", root.profileLabel
                )
                checkable: true
                Component.onCompleted: {
                    profileLoadCommandEditAction.checked = profileSettings.profileLoadCommand !== "";
                }
                onToggled: {
                    if (checked) {
                        profileLoadCommandEdit.forceActiveFocus();
                    } else {
                        profileSettings.profileLoadCommand = "";
                    }
                }
            }
            Kirigami.Action {
                id: profileUnloadCommandEditAction
                text: i18nc(
                    "@text:action:menu Script command to execute for power state (On AC Power, On Battery, ...)",
                    "When exiting \"%1\" state", root.profileLabel
                )
                checkable: true
                Component.onCompleted: {
                    profileUnloadCommandEditAction.checked = profileSettings.profileUnloadCommand !== "";
                }
                onToggled: {
                    if (checked) {
                        profileUnloadCommandEdit.forceActiveFocus();
                    } else {
                        profileSettings.profileUnloadCommand = "";
                    }
                }
            }
            Kirigami.Action {
                id: idleTimeoutCommandEditAction
                text: i18nc(
                    "@text:action:menu Script command to execute",
                    "After a period of inactivity"
                )
                checkable: true
                Component.onCompleted: {
                    idleTimeoutCommandEditAction.checked = (profileSettings.idleTimeoutCommand !== ""
                        // Always show field, regardless of addScriptCommandButton.visible.
                        || (kcm.supportedActions["RunScript"] === true && !kcm.supportsBatteryProfiles));
                }
                onToggled: {
                    if (checked) {
                        idleTimeoutCommandEdit.forceActiveFocus();
                    } else {
                        profileSettings.idleTimeoutCommand = "";
                    }
                }
            }
        }
    }

    RunScriptEdit {
        id: profileLoadCommandEdit
        Kirigami.FormData.label: i18nc(
            "@label:textfield Script command to execute for power state (On AC Power, On Battery, ...)",
            "When e&ntering \"%1\" state:", root.profileLabel
        )
        Accessible.name: i18nc(
            "@label:textfield for power state (On AC Power, On Battery, ...)",
            "Script command when entering \"%1\" state", root.profileLabel
        )
        visible: profileLoadCommandEditAction.checked
        Layout.fillWidth: true

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "ProfileLoadCommand"
        }
        command: profileSettings.profileLoadCommand
        onCommandChanged: {
            profileSettings.profileLoadCommand = command;
        }
        function resetToProfileSettings() {
            command = profileSettings.profileLoadCommand;
            profileLoadCommandEditAction.checked |= profileSettings.profileLoadCommand !== "";
        }
        Connections {
            target: root
            function onProfileSettingsChanged() { profileLoadCommandEdit.resetToProfileSettings(); }
        }
        Connections {
            target: profileSettings
            function onProfileLoadCommandChanged() { profileLoadCommandEdit.resetToProfileSettings(); }
        }
    }

    RunScriptEdit {
        id: profileUnloadCommandEdit
        Kirigami.FormData.label: i18nc(
            "@label:textfield Script command to execute for power state (On AC Power, On Battery, ...)",
            "When e&xiting \"%1\" state:", root.profileLabel
        )
        Accessible.name: i18nc(
            "@label:textfield for power state (On AC Power, On Battery, ...)",
            "Script command when exiting \"%1\" state", root.profileLabel
        )
        visible: profileUnloadCommandEditAction.checked
        Layout.fillWidth: true

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "ProfileUnloadCommand"
        }
        command: profileSettings.profileUnloadCommand
        onCommandChanged: {
            profileSettings.profileUnloadCommand = command;
        }
        function resetToProfileSettings() {
            command = profileSettings.profileUnloadCommand;
            profileUnloadCommandEditAction.checked |= profileSettings.profileUnloadCommand !== "";
        }
        Connections {
            target: root
            function onProfileSettingsChanged() { profileUnloadCommandEdit.resetToProfileSettings(); }
        }
        Connections {
            target: profileSettings
            function onProfileUnloadCommandChanged() { profileUnloadCommandEdit.resetToProfileSettings(); }
        }
    }

    RunScriptEdit {
        id: idleTimeoutCommandEdit
        Kirigami.FormData.label: i18nc(
            "@label:textfield Script command to execute",
            "After a period of inacti&vity:"
        )
        Accessible.name: i18nc("@@accessible:name:textfield", "Script command after a period of inactivity")

        visible: idleTimeoutCommandEditAction.checked
        Layout.fillWidth: true

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "IdleTimeoutCommand"
        }
        command: profileSettings.idleTimeoutCommand
        onCommandChanged: {
            profileSettings.idleTimeoutCommand = command;
        }
        function resetToProfileSettings() {
            command = profileSettings.idleTimeoutCommand;
            idleTimeoutCommandEditAction.checked |= profileSettings.idleTimeoutCommand !== "";
        }
        Connections {
            target: root
            function onProfileSettingsChanged() { idleTimeoutCommandEdit.resetToProfileSettings(); }
        }
        Connections {
            target: profileSettings
            function onIdleTimeoutCommandChanged() { idleTimeoutCommandEdit.resetToProfileSettings(); }
        }
    }

    TimeDurationComboBox {
        id: idleTimeoutCommandTimeDelayCombo
        Accessible.name: i18nc("@accessible:name:spinbox", "Period of inactivity until the script command executes")
        Layout.fillWidth: true
        visible: idleTimeoutCommandEdit.visible

        durationPromptLabel: i18nc("@label:spinbox After X minutes", "Execute script after:")
        durationPromptAcceptsUnits: [DurationPromptDialog.Unit.Seconds, DurationPromptDialog.Unit.Minutes]

        function translateSeconds(n, formatUnit = DurationPromptDialog.Unit.Seconds) {
            return formatUnit == DurationPromptDialog.Unit.Minutes
                ? translateMinutes(n / 60) : i18ncp("@option:combobox", "After %1 second", "After %1 seconds", n);
        }
        function translateMinutes(n) { return i18ncp("@option:combobox", "After %1 minute", "After %1 minutes", n); }

        valueRole: "seconds"
        textRole: "text"
        unitOfValueRole: DurationPromptDialog.Unit.Seconds
        durationPromptFromValue: 10
        presetOptions: [
                { seconds: 1 * 60, text: translateMinutes(1), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 2 * 60, text: translateMinutes(2), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 5 * 60, text: translateMinutes(5), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 10 * 60, text: translateMinutes(10), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 15 * 60, text: translateMinutes(15), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: 30 * 60, text: translateMinutes(30), unit: DurationPromptDialog.Unit.Minutes },
                { seconds: -1, text: i18nc("@option:combobox Choose a custom value outside the list of preset values", "Custom…") },
        ]
        customRequesterValue: -1
        configuredValue: profileSettings.runScriptIdleTimeoutSec
        configuredDisplayUnit: model[indexOfValue(configuredValue)]?.unit ?? DurationPromptDialog.Unit.Minutes

        onRegularValueActivated: { profileSettings.runScriptIdleTimeoutSec = currentValue; }
        onCustomDurationAccepted: {
            profileSettings.runScriptIdleTimeoutSec = valueToUnit(
                customDuration.value, customDuration.unit, DurationPromptDialog.Unit.Seconds);
        }

        onConfiguredValueOptionMissing: {
            const unit = configuredValue % 60 === 0
                ? customDuration?.unit ?? DurationPromptDialog.Unit.Minutes
                : DurationPromptDialog.Unit.Seconds;

            customOptions = [{
                seconds: configuredValue,
                text: translateSeconds(configuredValue, unit),
                unit: unit,
            }];
            customDuration = null;
        }

        KCM.SettingStateBinding {
            configObject: profileSettings
            settingName: "RunScriptIdleTimeoutSec"
        }
    }
}
