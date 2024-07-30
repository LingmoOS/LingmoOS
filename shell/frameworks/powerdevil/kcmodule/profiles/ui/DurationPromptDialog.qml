/*
    SPDX-FileCopyrightText: 2024 Kristen McWilliam <kmcwilliampublic@gmail.com>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.1-or-later

    Originating from kcm_screenlocker. Upstream any changes there until it goes into Kirigami (Addons).
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * A dialog that prompts the user to enter a duration.
 *
 * The dialog contains a spin box for the user to enter a duration, and its corresponding time unit.
 *
 * The dialog emits the `accepted` signal when the user clicks the confirm button, and the `rejected` signal when the user clicks the cancel button.
 *
 * After the dialog has been accepted, the `value` property will contain the duration value entered
 * by the user. The `unit` property will contain the time unit of `value`.
 */
Kirigami.Dialog {
    id: root

    /**
     * @brief The possible time units of the value of the input field.
     */
    enum Unit {
        Milliseconds,
        Seconds,
        Minutes,
        Hours,
        Days,
        Weeks,
        Months,
        Years
    }

    /**
     * @brief An array of time units that can be associated with the duration value.
     *
     * All elements must be `DurationPromptDialog.Unit` enum values, with no duplicates.
     *
     * At least one element is required. If more than one accepted unit is specified, the dialog
     * allows the user to select the unit of the duration.
     */
    required property var acceptsUnits

    /**
     * @brief A text label in front of the input field.
     */
    property string label

    /**
     * @brief The value of the input field.
     *
     * The value is in seconds if `unit` is `DurationPromptDialog.ValueType.Seconds`,
     * in minutes if `valueType` is `DurationPromptDialog.ValueType.Minutes`, and so on.
     *
     * Assign before opening the dialog to set the initial value.
     *
     * The value will have been updated when the user accepts the dialog.
     */
    property alias value: durationValueSpinBox.value

    property alias from: durationValueSpinBox.from
    property alias to: durationValueSpinBox.to

    /**
     * @brief The unit of the value of the input field, of type `DurationPromptDialog.Unit`.
     *
     * This must be one of the elements from the specified `acceptsUnits` property.
     * The first element of that array is used as default unit.
     */
    property int unit: acceptsUnits[0]

    padding: Kirigami.Units.largeSpacing
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
    showCloseButton: false

    // FIXME: KF 6.3 and up doesn't need content.implicitWidth as part of preferredWidth, it will
    //        automatically expand to implicit content size. Remove this once we can rely on KF 6.3.
    preferredWidth: Math.max(content.implicitWidth,
                             Kirigami.Units.gridUnit * 10, implicitHeaderWidth, implicitFooterWidth)

    onOpened: {
        // `focus: true` is not enough, because the OK button wants focus and gets priority.
        durationValueSpinBox.forceActiveFocus()
        // Set SpinBox width once, don't resize it if `to` changes based on unit changes.
        durationValueSpinBox.Layout.preferredWidth = durationValueSpinBox.implicitWidth;
    }

    RowLayout {
        id: content
        anchors.centerIn: parent

        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            id: labelItem
            visible: (root.label?.length ?? 0) > 0

            Kirigami.MnemonicData.enabled: visible && durationValueSpinBox.enabled
            Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.FormLabel
            Kirigami.MnemonicData.label: root.label ?? ""
            text: Kirigami.MnemonicData.richTextLabel
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.SpinBox {
                id: durationValueSpinBox
                from: 0
                to: 9999

                Shortcut {
                    sequence: labelItem.Kirigami.MnemonicData.sequence
                    onActivated: { durationValueSpinBox.forceActiveFocus(); }
                }
                Keys.onReturnPressed: event => root.accept()
            }

            function unitSuffixForValue(val, unit) {
                switch (unit) {
                case DurationPromptDialog.Unit.Milliseconds:
                    return i18ncp("The unit of the time input field", "millisecond", "milliseconds", val);
                case DurationPromptDialog.Unit.Seconds:
                    return i18ncp("The unit of the time input field", "second", "seconds", val);
                case DurationPromptDialog.Unit.Minutes:
                    return i18ncp("The unit of the time input field", "minute", "minutes", val);
                case DurationPromptDialog.Unit.Hours:
                    return i18ncp("The unit of the time input field", "hour", "hours", val);
                case DurationPromptDialog.Unit.Days:
                    return i18ncp("The unit of the time input field", "day", "days", val);
                case DurationPromptDialog.Unit.Weeks:
                    return i18ncp("The unit of the time input field", "week", "weeks", val);
                case DurationPromptDialog.Unit.Months:
                    return i18ncp("The unit of the time input field", "month", "months", val);
                case DurationPromptDialog.Unit.Years:
                    return i18ncp("The unit of the time input field", "year", "years", val);
                }
                console.warn("invalid unit in unitSuffixForValue()");
            }

            QQC2.Label {
                visible: acceptsUnits.length == 1
                text: parent.unitSuffixForValue(value, acceptsUnits[0])

                Layout.preferredWidth: Math.max(implicitWidth, pluralUnitLabelMetrics.advanceWidth + leftPadding + rightPadding)
                TextMetrics {
                    id: pluralUnitLabelMetrics
                    text: unitSelectionRadios.labelForUnit(acceptsUnits[0])
                }
            }

            ColumnLayout {
                id: unitSelectionRadios
                spacing: Kirigami.Units.smallSpacing
                visible: acceptsUnits.length > 1

                function labelForUnit(unit) {
                    switch (unit) {
                    case DurationPromptDialog.Unit.Milliseconds:
                        return i18nc("@text:radiobutton Unit of the time input field", "milliseconds");
                    case DurationPromptDialog.Unit.Seconds:
                        return i18nc("@text:radiobutton Unit of the time input field", "seconds");
                    case DurationPromptDialog.Unit.Minutes:
                        return i18nc("@text:radiobutton Unit of the time input field", "minutes");
                    case DurationPromptDialog.Unit.Hours:
                        return i18nc("@text:radiobutton Unit of the time input field", "hours");
                    case DurationPromptDialog.Unit.Days:
                        return i18nc("@text:radiobutton Unit of the time input field", "days");
                    case DurationPromptDialog.Unit.Weeks:
                        return i18nc("@text:radiobutton Unit of the time input field", "weeks");
                    case DurationPromptDialog.Unit.Months:
                        return i18nc("@text:radiobutton Unit of the time input field", "months");
                    case DurationPromptDialog.Unit.Years:
                        return i18nc("@text:radiobutton Unit of the time input field", "years");
                    }
                    console.warn("invalid unit in radioButtonLabelForValue()");
                }

                Repeater {
                    id: repeater
                    model: acceptsUnits

                    QQC2.RadioButton {
                        required property int index
                        required property int modelData

                        text: parent.labelForUnit(modelData)
                        checked: unit === modelData
                        onClicked: {
                            unit = modelData;
                        }

                        Keys.onReturnPressed: event => root.accept()
                        Keys.onUpPressed: event => {
                            const prev = repeater.itemAt(index - 1);
                            if (prev) {
                                prev.focus = true;
                            }
                        }
                        Keys.onDownPressed: event => {
                            const next = repeater.itemAt(index + 1);
                            if (next) {
                                next.focus = true;
                            }
                        }
                        Keys.onLeftPressed: event => {
                            if (!mirrored) {
                                durationValueSpinBox.forceActiveFocus();
                            }
                        }
                        Keys.onRightPressed: event => {
                            if (mirrored) {
                                durationValueSpinBox.forceActiveFocus();
                            }
                        }
                    }
                }
            }
        }
    }
}
