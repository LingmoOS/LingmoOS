/*
    SPDX-FileCopyrightText: 2024 Kristen McWilliam <kmcwilliampublic@gmail.com>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: LGPL-2.1-or-later

    Originating from kcm_screenlocker. Upstream any changes there until it goes into Kirigami (Addons).
*/

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

/**
 * A specialized ComboBox that displays a list of preset options plus a "Custom" option,
 * which invokes a parent-defined flow to let the user input a custom value. Once provided,
 * the custom value appears as an extra option at the bottom of the list.
 */
QQC2.ComboBox {
    id: root

    /**
     * Preset array of JS objects (or strings) with model data, to be used as base set of options for `model`.
     *
     * Include the custom requester option here, and set `customRequesterValue` accordingly.
     * Do not set `model` directly, assign this property instead.
     */
    required property var presetOptions

    /**
     * The latest valid value that has been selected.
     *
     * Changing this value will select the corresponding index. If no corresponding index exists,
     * the `configuredValueOptionMissing` signal will be emitted.
     */
    property var configuredValue: undefined

    /**
     * Emitted when `configuredValue` was not found in any element of `presetOptions`
     * or `customOptions`. To ensure a corresponding option exists, add an element to
     * `customOptions` with `configuredValue` in the `valueRole` role.
     *
     * @see customOptions
     */
    signal configuredValueOptionMissing

    /**
     * Emitted when a regular value option (i.e. any other than the custom requester option)
     * has been activated. Useful for accepting the new value without further checks.
     */
    signal regularValueActivated

    /**
     * This value used to find the option that will emit the `customRequest` signal when activated.
     *
     * This value is not usually persisted to configuration and only needed for consistency with
     * elements from `presetOptions`. Activation of any other option will emit `valueOptionActivated`.
     */
    property var customRequesterValue: undefined

    /**
     * Emitted when the custom value requester option was activated.
     *
     * Handle this by letting the user select the custom value (e.g. via modal dialog).
     * If the user selects a custom option, assign it to `configuredValue`. Always call
     * `customRequestCompleted()` at the end to ensure the correct index is set.
     */
    signal customRequest

    function customRequestCompleted() { updateIndex(); }

    /**
     * A model of elements to represent any additional custom values.
     *
     * The model follows the same format as the elements in `presetOptions`. All elements
     * will be appended to the preset options to constitute the combined `model`.
     *
     * This option is typically initialized with a single element containing `configuredValue`
     * in the `valueRole`, but the option list can remain unchanged even after `configuredValue`
     * has been updated.
     *
     * @see configuredValueOptionMissing
     */
    property var customOptions

    /** Exposed for completeness but not usually necessary. Will be set to -1 if no corresponding option has been found in presetOptions. */
    readonly property int customRequesterIndex: priv.customRequesterIndex

    QtObject {
        id: priv
        property bool initialized: false
        property int customRequesterIndex: -1
    }

    Component.onCompleted: {
        priv.initialized = true;
        updateOptions();
        updateCustomRequesterIndex();
        updateIndex();
    }
    onPresetOptionsChanged: {
        if (!priv.initialized) {
            return;
        }
        updateOptions();
        updateCustomRequesterIndex();
        updateIndex();
    }
    onCustomRequesterValueChanged: {
        if (!priv.initialized) {
            return;
        }
        updateCustomRequesterIndex();
        updateIndex();
    }
    onCustomOptionsChanged: {
        if (!priv.initialized) {
            return;
        }
        updateOptions();
        // Avoid recursion for `configuredValueOptionMissing` signals,
        // and also allow changing customOptions before setting configuredValue.
        const allowSignal = false;
        updateIndex(allowSignal);
    }
    onConfiguredValueChanged: {
        if (!priv.initialized) {
            return;
        }
        updateIndex();
    }

    onActivated: {
        if (currentIndex < 0) {
            return;
        }

        if (currentIndex === customRequesterIndex) {
            customRequest();
        } else {
            regularValueActivated();
        }
    }

    function updateOptions() {
        // Create a new array instance, to force a model update.
        model = [...(presetOptions ?? []), ...(customOptions ?? [])];
    }

    function updateCustomRequesterIndex() {
        priv.customRequesterIndex = indexOfValue(customRequesterValue);
    }

    function updateIndex(allowSignal = true) {
        if (configuredValue !== undefined && configuredValue !== currentValue) {
            const idx = indexOfValue(configuredValue);
            if (idx !== -1 && idx !== priv.customRequesterIndex && idx !== currentIndex) {
                currentIndex = idx;
            } else if (allowSignal) {
                configuredValueOptionMissing();
            }
        }
    }
}
