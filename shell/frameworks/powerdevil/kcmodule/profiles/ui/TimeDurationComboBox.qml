/*
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2

/**
 * A ComboBoxWithCustomValue for selecting durations represented by a single time unit.
 *
 * It invokes a `DurationPromptDialog` when the "Custom…" option is activated, automating the
 * conversion from option value to dialog properties and back. Time units from seconds up to weeks
 * are supported.
 */
ComboBoxWithCustomValue {
    id: root

    property string durationPromptLabel
    property string durationPromptTitle: i18nc("@title:window", "Custom Duration")

    /**
     * The list of time units accepted by the duration prompt, given as an array of
     * `DurationPromptDialog.Unit` values.
     */
    required property var durationPromptAcceptsUnits

    /**
     * The single time unit, given as `DurationPromptDialog.Unit`, that is associated to the
     * `valueRole` value of each item in the model. This unit is also used to interpret the
     * `configuredValue` property.
     *
     * For example, if `unitOfValueRole` is `DurationPromptDialog.Unit.Seconds`, then all model
     * item values will be considered as seconds and duration prompt inputs will be converted to
     * this unit accordingly.
     *
     * Generally you'll want to set this to the most fine-grained unit that the user is allowed
     * to assign/configure.
     */
    required property int unitOfValueRole

    /**
     * The time unit, given as `DurationPromptDialog.Unit`, that specifies how the
     * `configuredValue` property should be displayed to the user.
     *
     * For example, if `unitOfValueRole` is `DurationPromptDialog.Unit.Seconds` and
     * `configuredUnit` is `DurationPromptDialog.Unit.Minutes`, then the custom duration prompt
     * will display an initial value of 1 minute if the component's `configuredValue` is 60.
     */
    property int configuredDisplayUnit: durationPromptAcceptsUnits[0]

    /**
     * The lowest value that can be selected by the custom duration prompt, in units corresponding
     * to `unitOfValueRole`. The default value is 0.
     */
    property int durationPromptFromValue: 0

    /**
     * The highest value that can be selected by the custom duration prompt, in units corresponding
     * to `unitOfValueRole`. The default value is 24 hours.
     */
    property int durationPromptToValue: valueToUnit(1, DurationPromptDialog.Unit.Weeks, unitOfValueRole)

    /**
     * Will be set by this component when a custom duration prompt is confirmed.
     * If non-null, its properties include `unit` and `value`. Other than setting it, this
     * component does not make use of this property - feel free to set it back to null at any time.
     *
     * @see customDurationAccepted
     */
    property var customDuration: null

    /**
     * Emitted after the DurationPromptDialog has confirmed a custom duration value,
     * prior to calling `customValueRequestCompleted()` on the `ComboBoxWithCustomValue`
     * to finalize the request and reset the index.
     *
     * Update your setting (most likely bound to `configuredValue`) with the new `customDuration` object.
     *
     * @see customDuration
     */
    signal customDurationAccepted

    function valueToUnit(value, sourceUnit, targetUnit) {
        if (sourceUnit == targetUnit) {
            return value;
        }
        const seconds = sourceUnit === DurationPromptDialog.Unit.Seconds ? value
            : sourceUnit === DurationPromptDialog.Unit.Minutes ? (value * 60)
            : sourceUnit === DurationPromptDialog.Unit.Hours ? (value * 3600)
            : sourceUnit === DurationPromptDialog.Unit.Days ? (value * 3600 * 24)
            : sourceUnit === DurationPromptDialog.Unit.Weeks ? (value * 3600 * 24 * 7)
            : undefined;
        const result = targetUnit === DurationPromptDialog.Unit.Seconds ? seconds
            : targetUnit === DurationPromptDialog.Unit.Minutes ? (seconds / 60)
            : targetUnit === DurationPromptDialog.Unit.Hours ? (seconds / 3600)
            : targetUnit === DurationPromptDialog.Unit.Days ? (seconds / (3600 * 24))
            : targetUnit === DurationPromptDialog.Unit.Weeks ? (seconds / (3600 * 24 * 7))
            : undefined;
        if (result === undefined) {
            console.warn("Unsupported unit: TimeDurationComboBox only supports conversions between seconds, minutes, hours, days and weeks. Months and years are not supported as conversion factors are not constant.");
        }
        return result;
    }

    onCustomRequest: {
        // Pass the configured value to the dialog so it can be pre-filled in the input field.
        // Anything 0 or below is not a reasonable timeout and should be reserved for special values.
        customDurationPromptDialogLoader.sourceComponent = durationPromptDialogComponent;
        customDurationPromptDialogLoader.item.unit = configuredDisplayUnit;
        customDurationPromptDialogLoader.item.value = // `from` and `to` will clamp this value
            valueToUnit(configuredValue, unitOfValueRole, configuredDisplayUnit)
        customDurationPromptDialogLoader.item.open();
        customDurationPromptDialogLoader.item.forceActiveFocus();
    }

    /// Dialog handled by a Loader to avoid loading it until it is needed.
    Loader {
        id: customDurationPromptDialogLoader
        anchors.centerIn: parent
    }
    Component {
        id: durationPromptDialogComponent

        DurationPromptDialog {
            title: root.durationPromptTitle
            label: root.durationPromptLabel
            parent: root.QQC2.Overlay.overlay

            acceptsUnits: root.durationPromptAcceptsUnits
            from: Math.ceil(root.valueToUnit(root.durationPromptFromValue, root.unitOfValueRole, unit))
            to: Math.floor(root.valueToUnit(root.durationPromptToValue, root.unitOfValueRole, unit))

            onAccepted: {
                // Set the combo box's customUnit prior to configuredValue,
                // so the selected unit is set explicitly instead of guessed by modulo.
                root.customDuration = {value: value, unit: unit};
                root.customDurationAccepted();
                root.customRequestCompleted();
                root.forceActiveFocus();
            }
            onRejected: {
                root.customDuration = null;
                root.customRequestCompleted();
                root.forceActiveFocus();
            }
        }
    }
}
