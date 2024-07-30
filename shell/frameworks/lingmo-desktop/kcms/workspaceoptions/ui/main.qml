/*
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.kcmutils as KCM
import org.kde.kwindowsystem

KCM.SimpleKCM {
    implicitWidth: LingmoUI.Units.gridUnit * 40

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: LingmoUI.InlineMessage {
        id: primarySelectionRebootMessage
        position: LingmoUI.InlineMessage.Position.Header
        type: LingmoUI.MessageType.Information
        text: i18n("The system must be restarted before changes to the middle-click paste setting can take effect.")
        visible: false
        showCloseButton: true
        actions: [
            LingmoUI.Action {
                icon.name: "system-reboot"
                text: i18n("Restart")
                onTriggered: kcm.requestReboot();
            }
        ]
        Connections {
            target: kcm
            function onPrimarySelectionOptionSaved() {
                primarySelectionRebootMessage.visible = true;
            }
        }
    }

    LingmoUI.FormLayout {

        // Visual behavior settings
        QQC2.CheckBox {
            LingmoUI.FormData.label: i18n("Visual behavior:")
            text: i18n("Display informational tooltips on mouse hover")
            checked: kcm.lingmoSettings.delay > 0
            onCheckedChanged: kcm.lingmoSettings.delay = (checked ? 0.7 : -1)

            KCM.SettingStateBinding {
                configObject: kcm.lingmoSettings
                settingName: "delay"
            }
        }

        QQC2.CheckBox {
            text: i18n("Display visual feedback for status changes")
            checked: kcm.lingmoSettings.osdEnabled
            onCheckedChanged: kcm.lingmoSettings.osdEnabled = checked

            KCM.SettingStateBinding {
                configObject: kcm.lingmoSettings
                settingName: "osdEnabled"
            }
        }

        Item {
            LingmoUI.FormData.isSection: false
        }

        // We want to show the slider in a logarithmic way. ie
        // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
        // 0 is a special case, which means "instant speed / no animations"
        ColumnLayout {
            LingmoUI.FormData.label: i18n("Animation speed:")
            LingmoUI.FormData.buddyFor: slider
            Layout.fillWidth: true
            spacing: LingmoUI.Units.smallSpacing

            QQC2.Slider {
                id: slider
                Layout.fillWidth: true
                from: -4
                to: 4
                stepSize: 0.5
                snapMode: QQC2.Slider.SnapAlways
                onMoved: kcm.globalsSettings.animationDurationFactor =
                    (value === to) ? 0 : (1.0 / Math.pow(2, value))
                value: (kcm.globalsSettings.animationDurationFactor === 0)
                    ? slider.to
                    : -(Math.log(kcm.globalsSettings.animationDurationFactor) / Math.log(2))

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "animationDurationFactor"
                }
            }
            RowLayout {
                spacing: 0
                QQC2.Label {
                    text: i18nc("Animation speed", "Slow")
                    textFormat: Text.PlainText
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("Animation speed", "Instant")
                    textFormat: Text.PlainText
                }
            }
        }

        Item {
            LingmoUI.FormData.isSection: false
        }

        RowLayout {
            LingmoUI.FormData.label: i18nc("@label", "Scrolling:")
            spacing: 0

            QQC2.CheckBox {
                text: i18nc("@option:check", "Prefer smooth scrolling")
                checked: kcm.globalsSettings.smoothScroll
                onToggled: kcm.globalsSettings.smoothScroll = checked

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "smoothScroll"
                }
            }
            LingmoUI.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "This setting enables or disables animated transitions when scrolling with a mouse wheel or the keyboard. Some applications may not honor this setting because they either do not support smooth scrolling, or have their own setting for enabling and disabling it.")
            }
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        // Click behavior settings

        QQC2.ButtonGroup { id: singleClickGroup }

        ColumnLayout {
            LingmoUI.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
            LingmoUI.FormData.buddyFor: doubleClick

            spacing: 0

            QQC2.RadioButton {
                id: doubleClick
                text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
                checked: !kcm.globalsSettings.singleClick
                onToggled: kcm.globalsSettings.singleClick = false
                QQC2.ButtonGroup.group: singleClickGroup

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "singleClick"
                    extraEnabledConditions: singleClick.enabled
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                leftPadding: singleClick.indicator.width
                text: i18n("Open by double-clicking instead")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: LingmoUI.Theme.smallFont
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: singleClick
                text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
                checked: kcm.globalsSettings.singleClick
                onToggled: kcm.globalsSettings.singleClick = true
                QQC2.ButtonGroup.group: singleClickGroup

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "singleClick"
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                leftPadding: singleClick.indicator.width
                text: i18n("Select by clicking on item's selection marker")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: LingmoUI.Theme.smallFont
            }
        }

        Item {
            LingmoUI.FormData.isSection: false
        }

        // scroll handle settings

        QQC2.ButtonGroup { id: scrollHandleBehaviorGroup }

        QQC2.RadioButton {
            LingmoUI.FormData.label: i18n("Clicking in scrollbar track:")
            text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls to the clicked location'", "Scrolls to the clicked location")
            checked: !kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
            onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = false
            QQC2.ButtonGroup.group: scrollHandleBehaviorGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "scrollbarLeftClickNavigatesByPage"
                extraEnabledConditions: scrollbarLeftClickNavigatesByPage.enabled
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: scrollbarLeftClickNavigatesByPage
                text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls one page up or down'", "Scrolls one page up or down")
                checked: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
                onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = true
                QQC2.ButtonGroup.group: scrollHandleBehaviorGroup

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "scrollbarLeftClickNavigatesByPage"
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                leftPadding: scrollbarLeftClickNavigatesByPage.indicator.width
                text: i18n("Middle-click to scroll to clicked location")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: LingmoUI.Theme.smallFont
            }
        }

        Item {
            LingmoUI.FormData.isSection: true
            visible: primarySelectionRadio.visible
        }

        QQC2.CheckBox {
            id: primarySelectionRadio
            LingmoUI.FormData.label: i18nc("@radio part of a complete sentence: 'Middle click pastes selected text'", "Middle Click:")
            visible: KWindowSystem.isPlatformWayland
            text: i18nc("@radio part of a complete sentence: 'Middle click pastes selected text'", "Pastes selected text")
            checked: kcm.kwinSettings.primarySelection
            onToggled: kcm.kwinSettings.primarySelection = checked

            KCM.SettingStateBinding {
                configObject: kcm.kwinSettings
                settingName: "primarySelection"
            }
        }

        Item {
            LingmoUI.FormData.isSection: true
        }

        QQC2.ButtonGroup { id: tabletModeBehaviorGroup }

        RowLayout {
            LingmoUI.FormData.label: i18n("Touch Mode:")
            QQC2.RadioButton {
                text: KWindowSystem.isPlatformWayland ? i18nc("As in: 'Touch Mode is automatically enabled as needed'", "Automatically enable as needed") : i18nc("As in: 'Touch Mode is never enabled'", "Never enabled")
                checked: kcm.kwinSettings.tabletMode === "auto"
                onToggled: {
                    if (checked) {
                        kcm.kwinSettings.tabletMode = "auto"
                    }
                }
                QQC2.ButtonGroup.group: tabletModeBehaviorGroup

                KCM.SettingStateBinding {
                    configObject: kcm.kwinSettings
                    settingName: "tabletMode"
                }
            }
            LingmoUI.ContextualHelpButton {
                visible: KWindowSystem.isPlatformWayland
                toolTipText: i18n("Touch Mode will be automatically activated whenever the system detects a touchscreen but no mouse or touchpad. For example: when a transformable laptop's keyboard is flipped around or detached.")
            }
        }

        QQC2.RadioButton {
            text: i18nc("As in: 'Touch Mode is always enabled'", "Always enabled")
            checked: kcm.kwinSettings.tabletMode === "on"
            onToggled: {
                if (checked) {
                    kcm.kwinSettings.tabletMode = "on"
                }
            }
            QQC2.ButtonGroup.group: tabletModeBehaviorGroup

            KCM.SettingStateBinding {
                configObject: kcm.kwinSettings
                settingName: "tabletMode"
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: touchModeAlwaysOffRadioButton
                visible: KWindowSystem.isPlatformWayland
                text: i18nc("As in: 'Touch Mode is never enabled'", "Never enabled")
                checked: kcm.kwinSettings.tabletMode === "off"
                onToggled: {
                    if (checked) {
                        kcm.kwinSettings.tabletMode = "off"
                    }
                }
                QQC2.ButtonGroup.group: tabletModeBehaviorGroup

                KCM.SettingStateBinding {
                    configObject: kcm.kwinSettings
                    settingName: "tabletMode"
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                Layout.preferredWidth: LingmoUI.Units.gridUnit * 20
                leftPadding: touchModeAlwaysOffRadioButton.indicator.width
                text: i18n("In Touch Mode, many elements of the user interface will become larger to more easily accommodate touch interaction.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: LingmoUI.Theme.smallFont
                wrapMode: Text.WordWrap
            }
        }

        // There is no label for what middle-clicking does when using the
        // "click to zoom the handle" behavior because Qt doesn't invert the
        // middle-click functionality when using this; see
        // https://bugreports.qt.io/browse/QTBUG-80728

        Item {
            LingmoUI.FormData.isSection: true
        }

        RowLayout {
            LingmoUI.FormData.label: i18n("Double-click interval:")
            spacing: LingmoUI.Units.smallSpacing

            QQC2.SpinBox {
                id: spinbox
                from: 100
                to: 2000
                stepSize: 100

                validator: IntValidator {
                    bottom: Math.min(spinbox.from, spinbox.to)
                    top: Math.max(spinbox.from, spinbox.to)
                }

                textFromValue: (value, locale) => {
                    return i18np("%1 msec", "%1 msec", value)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18n("msec"), ""))
                }

                onValueModified: kcm.globalsSettings.doubleClickInterval = value

                value: kcm.globalsSettings.doubleClickInterval

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "doubleClickInterval"
                }
            }

            LingmoUI.ContextualHelpButton {
                toolTipText: i18n("Two clicks within this duration are considered a double-click. Some applications may not honor this setting.")
            }
        }
    }
}
