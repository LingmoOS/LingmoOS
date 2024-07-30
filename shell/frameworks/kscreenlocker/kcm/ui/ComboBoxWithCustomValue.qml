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
 * A specialized ComboBox that assists in displaying a list of preset options plus a "Custom" option,
 * which invokes a parent-defined flow to let the user input a custom value. One or more
 * custom values can then be added to the `model` by the parent.
 *
 * In addition to the standard ComboBox `model` property, this component wants its user to set
 * two value properties that refer to one of the elements in `model`:
 *
 * - `configuredValue`, if set, will automatically update the ComboBox index to the corresponding
 *   option whenever this value itself or the `model` changes. If no corresponding option exists,
 *   the `configuredValueOptionMissing` signal will be emitted so the component user can easily
 *   add one to the `model`.
 * - `customRequesterValue`, if set, will provide additional signals named `regularValueActivated`
 *    and `customRequest` to conveniently handle simple selection updates vs. custom value selection
 *    respectively.
 *
 * Both of these properties are optional, but together they provide a convenient pattern to
 * implement item selection with custom value options.
 */
QQC2.ComboBox {
    id: root

    /**
     * The latest valid value that has been selected. Set this instead of `currentIndex`.
     *
     * Changing this value will select the corresponding index. If no corresponding index exists,
     * the `configuredValueOptionMissing` signal will be emitted.
     *
     * @see configuredValueOptionMissing
     */
    property var configuredValue: undefined

    /**
     * Emitted when `configuredValue` was not found in any element of `model`. To ensure that a
     * corresponding option exists, add an element with `configuredValue` in the `valueRole` role.
     *
     * @see configuredValue
     */
    signal configuredValueOptionMissing()

    /**
     * The value corresponding to the custom requester option which will emit the `customRequest`
     * signal when activated.
     *
     * This value is not usually persisted to configuration.
     * Activation of any other option will emit `regularValueActivated` instead.
     *
     * @see customRequest
     * @see regularValueActivated
     */
    property var customRequesterValue: undefined

    /**
     * Emitted when a regular value option (i.e. any other than the custom requester option)
     * has been activated.
     *
     * You can use this instead of the `activated` signal without manually checking against
     * `customRequesterValue`.
     *
     * @see customRequesterValue
     */
    signal regularValueActivated()

    /**
     * Emitted when the custom value requester option was activated.
     *
     * Handle this by letting the user select the custom value (e.g. via modal dialog).
     * If the user selects a custom option, assign it to `configuredValue`. Always call
     * `customRequestCompleted()` at the end to ensure the correct index is set.
     *
     * @see customRequesterValue
     * @see customRequestCompleted
     */
    signal customRequest()

    function customRequestCompleted(): void {
        updateIndex();
    }

    /** private */
    property bool __initialized: false
    /** private */
    property bool __allowSignal: true

    Component.onCompleted: {
        __initialized = true;
        updateIndex();
    }
    onModelChanged: { updateIndex(); }
    onCustomRequesterValueChanged: { updateIndex(); }
    onConfiguredValueChanged: { updateIndex(); }

    onActivated: {
        if (customRequesterValue === undefined || currentIndex < 0) {
            return;
        }

        if (currentValue === customRequesterValue) {
            customRequest();
        } else {
            regularValueActivated();
        }
    }

    /**
     * Try to set currentIndex to the option with `configuredValue` in its value role.
     * If no such option exists in the current `model`, the `configuredValueOptionMissing` signal
     * is emitted instead.
     *
     * You shouldn't usually have to call this function manually, but it is available for cases
     * when this component does not pick up on certain changes in the model.
     */
    function updateIndex(): void {
        if (!__initialized) {
            return;
        }
        if (configuredValue === undefined || configuredValue === currentValue || configuredValue === customRequesterValue) {
            return;
        }

        const idx = indexOfValue(configuredValue);
        if (idx !== -1) {
            currentIndex = idx;
        } else if (__allowSignal) {
            // Avoid recursion for `configuredValueOptionMissing` signals.
            __allowSignal = false;
            configuredValueOptionMissing();
            __allowSignal = true;
        }
    }
}
