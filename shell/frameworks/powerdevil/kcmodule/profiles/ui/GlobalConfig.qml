/*
 *  SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title", "Advanced Power Settings")

    readonly property var globalSettings: kcm.settings.global
    readonly property var externalSettings: kcm.externalServiceSettings

    readonly property string percentageTemplate: i18nc("Percentage value example, used for formatting battery levels in the power management settings", "10%")
    readonly property var percentageExtractor: new RegExp(
        percentageTemplate
            // Make all the non-number, non-whitespace optional.
            .replace(/[^\d\s]/g, function(matched) { return matched + "?"; })
            // Replace the concrete number from percentageTemplate with a number expression.
            .replace(/\s*\d+\s*/, "\\s*(\\d+)\\s*")
    )

    RegularExpressionValidator {
        id: percentageValidator
        regularExpression: percentageExtractor
    }

    function formatPercent(value, locale) {
        return percentageTemplate.replace(/[0-9]+/, Number(value).toLocaleString(locale, 'f', 0));
    }
    function extractPercent(text, locale) {
        const match = text.match(root.percentageExtractor);
        return match[1] ?? Number.fromLocaleString(locale, match[1]);
    }

    readonly property int maxImplicitWidthOfSpinBoxes: Math.max(
        batteryLowSpin.implicitWidth,
        batteryCriticalSpin.implicitWidth,
        peripheralBatteryLowSpin.implicitWidth,
        chargeStopThresholdSpin.implicitWidth,
        chargeStartThresholdSpin.implicitWidth
    )

    Kirigami.FormLayout {
        anchors.left: parent.left
        anchors.right: parent.right

        Item {
            id: batteryLevelsHeader
            Kirigami.FormData.label: i18nc("@title:group", "Battery Levels")
            Kirigami.FormData.isSection: true
            visible: batteryLowSpin.visible || batteryCriticalSpin.visible || batteryCriticalCombo.visible || peripheralBatteryLowSpin.visible
        }

        RowLayout {
            Kirigami.FormData.label: i18nc(
                "@label:spinbox Low battery level percentage for the main power supply battery",
                "&Low level:"
            )
            Kirigami.FormData.buddyFor: batteryLowSpin
            visible: kcm.isPowerSupplyBatteryPresent
            spacing: Kirigami.Units.smallSpacing

            QQC2.SpinBox {
                id: batteryLowSpin
                Accessible.name: i18nc("@accessible:name:spinbox", "Low battery level")
                Accessible.description: batteryLowContextualHelp.toolTipText
                Layout.preferredWidth: maxImplicitWidthOfSpinBoxes
                from: 0
                to: 100

                KCM.SettingStateBinding {
                    configObject: globalSettings
                    settingName: "BatteryLowLevel"
                }
                value: globalSettings.batteryLowLevel
                onValueModified: {
                    globalSettings.batteryLowLevel = value;
                    // In Qt 6.6, SpinBox breaks the value binding on keyboard input. Restore it again.
                    value = Qt.binding(() => globalSettings.batteryLowLevel);
                }

                editable: true
                validator: percentageValidator
                textFromValue: formatPercent
                valueFromText: extractPercent
            }

            Kirigami.ContextualHelpButton {
                id: batteryLowContextualHelp
                toolTipText: i18nc("@info:whatsthis", "The battery charge will be considered low when it drops to this level. Settings for low battery will be used instead of regular battery settings.")
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18nc(
                "@label:spinbox Critical battery level percentage for the main power supply battery",
                "Cr&itical level:"
            )
            Kirigami.FormData.buddyFor: batteryCriticalSpin
            visible: kcm.isPowerSupplyBatteryPresent
            spacing: Kirigami.Units.smallSpacing

            QQC2.SpinBox {
                id: batteryCriticalSpin
                Accessible.name: i18nc("@accessible:name:spinbox", "Critical battery level")
                Accessible.description: batteryCriticalContextualHelp.toolTipText
                Layout.preferredWidth: maxImplicitWidthOfSpinBoxes
                from: 0
                to: batteryLowSpin.value

                KCM.SettingStateBinding {
                    configObject: globalSettings
                    settingName: "BatteryCriticalLevel"
                }
                value: globalSettings.batteryCriticalLevel
                onValueModified: {
                    globalSettings.batteryCriticalLevel = value;
                    // In Qt 6.6, SpinBox breaks the value binding on keyboard input. Restore it again.
                    value = Qt.binding(() => globalSettings.batteryCriticalLevel);
                }

                editable: true
                validator: percentageValidator
                textFromValue: formatPercent
                valueFromText: extractPercent
            }

            Kirigami.ContextualHelpButton {
                id: batteryCriticalContextualHelp
                toolTipText: i18nc("@info:whatsthis", "The battery charge will be considered critical when it drops to this level. After a brief warning, the system will automatically suspend or shut down, according to the configured critical battery level action.")
            }
        }

        Kirigami.InlineMessage {
            Kirigami.FormData.isSection: true
            visible: batteryCriticalCombo.visible && !batteryCriticalCombo.isConfiguredValueSupported
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18nc("@info:status", "The action you had previously configured for reaching the critical battery level is now unsupported on your system. Please select a different one.")
        }

        ComboBoxWithIcon {
            id: batteryCriticalCombo
            Kirigami.FormData.label: i18nc(
                "@label:combobox Power action such as sleep/hibernate that will be executed when the critical battery level is reached",
                "A&t critical level:"
            )
            Accessible.name: i18nc("@accessible:name:combobox", "Action performed at critical battery level")
            visible: batteryCriticalSpin.visible

            model: kcm.batteryCriticalActionModel
            textRole: "name"
            valueRole: "value"

            readonly property bool isConfiguredValueSupported: currentValue === globalSettings.batteryCriticalAction

            KCM.SettingStateBinding {
                configObject: globalSettings
                settingName: "BatteryCriticalAction"
            }
            Component.onCompleted: {
                // indexOfValue() is invalid before onCompleted, so wait until here to bind currentIndex.
                currentIndex = Qt.binding(() => indexOfValue(globalSettings.batteryCriticalAction));
            }
            onActivated: {
                globalSettings.batteryCriticalAction = currentValue;
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18nc(
                "@label:spinbox Low battery level percentage for peripheral devices",
                "Low level for peripheral d&evices:"
            )
            Kirigami.FormData.buddyFor: peripheralBatteryLowSpin
            visible: kcm.isPeripheralBatteryPresent
            spacing: Kirigami.Units.smallSpacing

            QQC2.SpinBox {
                id: peripheralBatteryLowSpin
                Accessible.name: i18nc("@accessible:name:spinbox", "Low battery level for peripheral devices")
                Accessible.description: peripheralBatteryLowContextualHelp.toolTipText
                Layout.preferredWidth: maxImplicitWidthOfSpinBoxes
                from: 0
                to: 100

                KCM.SettingStateBinding {
                    configObject: globalSettings
                    settingName: "PeripheralBatteryLowLevel"
                }
                value: globalSettings.peripheralBatteryLowLevel
                onValueModified: {
                    globalSettings.peripheralBatteryLowLevel = value;
                    // In Qt 6.6, SpinBox breaks the value binding on keyboard input. Restore it again.
                    value = Qt.binding(() => globalSettings.peripheralBatteryLowLevel);
                }

                editable: true
                validator: percentageValidator
                textFromValue: formatPercent
                valueFromText: extractPercent
            }

            Kirigami.ContextualHelpButton {
                id: peripheralBatteryLowContextualHelp
                toolTipText: i18nc("@info:whatsthis", "The battery charge for peripheral devices will be considered low when it reaches this level.")
            }
        }

        /////

        Item {
            id: chargeLimitHeader
            Kirigami.FormData.label: i18nc("@title:group", "Charge Limit")
            Kirigami.FormData.isSection: true
            visible: chargeStopThresholdSpin.visible || chargeStartThresholdSpin.visible || batteryConservationModeCheck.visible
        }

        QQC2.CheckBox {
            id: batteryConservationModeCheck
            visible: kcm.isBatteryConservationModeSupported
            checked: externalSettings.batteryConservationMode
            onToggled: externalSettings.batteryConservationMode = checked

            Kirigami.FormData.label: i18nc("@label:checkbox", "&Battery protection:")
            text: i18nc("@text:checkbox", "Limit the maximum battery charge")
            Accessible.name: text
        }

        QQC2.SpinBox {
            id: chargeStopThresholdSpin
            Kirigami.FormData.label: i18nc(
                "@label:spinbox Battery will stop charging when this charge percentage is reached",
                "&Stop charging at:"
            )
            Layout.preferredWidth: maxImplicitWidthOfSpinBoxes
            visible: kcm.isChargeStopThresholdSupported
            from: 50
            to: 100

            value: externalSettings.chargeStopThreshold
            onValueModified: {
                if (kcm.isChargeStopThresholdSupported) {
                    externalSettings.chargeStopThreshold = value;
                }
                // In Qt 6.6, SpinBox breaks the value binding on keyboard input. Restore it again.
                value = Qt.binding(() => externalSettings.chargeStopThreshold);
            }

            editable: true
            validator: percentageValidator
            textFromValue: formatPercent
            valueFromText: extractPercent
        }

        QQC2.SpinBox {
            id: chargeStartThresholdSpin
            Kirigami.FormData.label: i18nc(
                "@label:spinbox Battery will start charging again when this charge percentage is reached, after having hit the stop-charging threshold earlier",
                "Start charging once &below:"
            )
            Layout.preferredWidth: maxImplicitWidthOfSpinBoxes
            visible: kcm.isChargeStartThresholdSupported
            from: 1
            to: chargeStopThresholdSpin.value // max value == "always charge right up to charge limit"

            value: externalSettings.chargeStartThreshold > 0 ? externalSettings.chargeStartThreshold : to;

            function setChargeStartThreshold() {
                if (kcm.isChargeStartThresholdSupported) {
                    externalSettings.chargeStartThreshold = value < to ? value : 0;
                }
                // In Qt 6.6, SpinBox breaks the value binding on keyboard input.
                // We do too, in onToChanged. Restore it again.
                value = Qt.binding(() => externalSettings.chargeStartThreshold > 0 ? externalSettings.chargeStartThreshold : to);
            }
            property int lockstepUpperBound: -1 // tracks externalSettings and manual user changes, not range limit changes

            onValueModified: {
                setChargeStartThreshold();
                lockstepUpperBound = value;
            }
            onToChanged: {
                // Follow the stop threshold back up until the value we started from.
                if (externalSettings.chargeStartThreshold > 0 && to == value) {
                    lockstepUpperBound = value;
                }
                else if (externalSettings.chargeStartThreshold == 0 && to <= lockstepUpperBound) {
                    value = to;
                }
                setChargeStartThreshold();
            }
            Connections {
                target: externalSettings
                function onChargeStartThresholdChanged() {
                    if (externalSettings.chargeStartThreshold == 0) {
                        chargeStartThresholdSpin.lockstepUpperBound = chargeStartThresholdSpin.to;
                    }
                }
            }
            Component.onCompleted: {
                chargeStartThresholdSpin.lockstepUpperBound = value;
            }

            editable: true
            validator: percentageValidator
            textFromValue: formatPercent
            valueFromText: extractPercent
        }

        Kirigami.InlineMessage {
            id: chargeStopThresholdReconnectMessage
            Kirigami.FormData.isSection: true
            visible: chargeLimitHeader.visible && kcm.chargeStopThresholdMightNeedReconnect
            implicitWidth: batteryThresholdExplanation.implicitWidth
            type: Kirigami.MessageType.Warning
            text: i18nc("@info:status", "You might have to disconnect and re-connect the power source to start charging the battery again.")
        }

        Kirigami.InlineMessage {
            id: batteryThresholdExplanation
            Kirigami.FormData.isSection: true
            // iFixit suggests keeping the charge between 40-80%, Battery University lists 25-85% as a decent tradeoff.
            // Show this reminder only when high charge thresholds are configured.
            visible: (chargeLimitHeader.visible && !chargeStopThresholdReconnectMessage.visible && chargeStopThresholdSpin.value > 85)
                  || (batteryConservationModeCheck.visible && !batteryConservationModeCheck.checked)
            implicitWidth: Kirigami.Units.gridUnit * 16
            text: i18nc("@info", "Regularly charging the battery close to 100%, or fully discharging it, may accelerate deterioration of battery health. By limiting the maximum battery charge, you can help extend the battery lifespan.")
        }

        /////

        Item {
            Kirigami.FormData.label: i18nc(
                "@title:group Miscellaneous power management settings that didn't fit elsewhere",
                "Other Settings"
            )
            Kirigami.FormData.isSection: true
            // If these are the only settings, they're not "Other Settings" and we don't need a section title
            visible: batteryLevelsHeader.visible || chargeLimitHeader.visible
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18nc("@label:checkbox", "&Media playback:")
            text: i18nc("@text:checkbox", "Pause media players when suspending")
            Accessible.name: text

            KCM.SettingStateBinding {
                configObject: globalSettings
                settingName: "PausePlayersOnSuspend"
            }
            checked: globalSettings.pausePlayersOnSuspend
            onToggled: { globalSettings.pausePlayersOnSuspend = checked; }
        }

        ColumnLayout {
            id: relatedPagesLayout
            Kirigami.FormData.label: i18nc("@label:button", "Related pages:")
            Kirigami.FormData.buddyFor: firstRelatedPage
            spacing: 0

            readonly property real maxPageButtonImplicitWidth: Math.max(
                firstRelatedPage.implicitWidth,
                desktopSessionsPage.implicitWidth,
                activitiesPage.implicitWidth,
            )

            MostUsedItem {
                id: firstRelatedPage
                kcmIcon: "preferences-desktop-notification-bell"
                kcmName: i18nc(
                    "@text:button Name of KCM, plus Power Management notification category",
                    "Notifications: Power Management"
                )
                Accessible.name: i18n("Open \"Notifications\" settings page, \"Power Management\" section")
                Layout.preferredWidth: relatedPagesLayout.maxPageButtonImplicitWidth
                onClicked: KCM.KCMLauncher.openSystemSettings("kcm_notifications", "--notifyrc=powerdevil")
            }
            MostUsedItem {
                id: desktopSessionsPage
                kcmIcon: "system-log-out"
                kcmName: i18nc("@text:button Name of KCM", "Desktop Session")
                Accessible.name: i18n("Open \"Desktop Session\" settings page")
                Layout.preferredWidth: relatedPagesLayout.maxPageButtonImplicitWidth
                onClicked: KCM.KCMLauncher.openSystemSettings("kcm_smserver")
            }
            MostUsedItem {
                id: activitiesPage
                kcmIcon: "preferences-desktop-activities"
                kcmName: i18nc("@text:button Name of KCM", "Activities")
                Accessible.name: i18n("Open \"Activities\" settings page")
                Layout.preferredWidth: relatedPagesLayout.maxPageButtonImplicitWidth
                onClicked: KCM.KCMLauncher.openSystemSettings("kcm_activities")
            }
        }
    }
}
