/*
 *   SPDX-FileCopyrightText: 2014 Nikita Skovoroda <chalkerx@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

namespace PowerDevil
{
class BrightnessLogic
{
public:
    BrightnessLogic();
    virtual ~BrightnessLogic() = default;

    /**
     * This enum defines different types of discrete brightness adjustment actions.
     * These will usually get triggered by brightness keys or scroll wheel events.
     *
     * - Increase: Increase brightness by one step (Qt::Key_MonBrightnessUp or Qt::Key_KeyboardBrightnessUp)
     * - Decrease: Decrease brightness by one step (Qt::Key_MonBrightnessDown or Qt::Key_KeyboardBrightnessDown)
     * - IncreaseSmall: Increase brightness by a small step such as 1% (Qt::Key_MonBrightnessUp or Qt::Key_KeyboardBrightnessUp + Qt::ShiftModifier)
     * - DecreaseSmall: Decrease brightness by a small step such as 1% (Qt::Key_MonBrightnessDown or Qt::Key_KeyboardBrightnessDown + Qt::ShiftModifier)
     */
    enum StepAdjustmentAction {
        Increase,
        Decrease,
        IncreaseSmall,
        DecreaseSmall,
    };

    /**
     * This struct contains information about current brightness state for a single device
     */
    struct BrightnessInfo {
        /** The raw brightness value, from 0 to valueMax */
        int value = -1;
        /**
         * The minimum practical brightness value for this device. Usually 0, but should be
         * overridden for device types where a higher minimum value brightness makes sense.
         * For example, for screens where a value of 0 sometimes turns off the backlight
         * completely, which is not wanted there.
         */
        int valueMin = -1;
        /** The maximum possible brightness value for this device */
        int valueMax = -1;
        /** The maximum possible brightness step for this device */
        int steps = -1;
    };

    /**
     * Sets the current brightness value.
     *
     * @param value Raw brightness value
     */
    void setValue(int value);

    /**
     * Sets the minimum and maximum brightness value.
     *
     * @param valueMin Minimum brightness value
     * @param valueMax Maximum brightness value
     */
    void setValueRange(int valueMin, int valueMax);

    /**
     * Calculate new brightness value that should be set by a discrete brightness adjustment action.
     *
     * @param adjustment The adjustment action, defining the step size and direction.
     * @return The brightness value that the action should set, or -1 if nothing should be done
     */
    int adjusted(StepAdjustmentAction adjustment) const;

    /**
     * Calculates the brightness value of the closest step upwards.
     * (Closest step that is higher than current brightness value).
     *
     * @return The brightness value of the closest step upwards
     */
    int increased() const;

    /**
     * Calculates the brightness value of the closest step downwards.
     * (Closest step that is lower than current brightness value).
     *
     * @return The brightness value of the closest step downwards
     */
    int decreased() const;

    /**
     * Calculates the brightness value of the closest step upwards.
     * (Closest step that is higher than current brightness value).
     *
     * @return The brightness value of the closest step upwards
     */
    int increasedSmall() const;

    /**
     * Calculates the brightness value of the closest step downwards.
     * (Closest step that is lower than current brightness value).
     *
     * @return The brightness value of the closest step downwards
     */
    int decreasedSmall() const;

    /**
     * Retrieve the maximum possible brightness step for this instance
     *
     * @return Maximum possible brightness step
     */
    int steps() const;

    /**
     * Retrieve the supplied brightness value expressed as a percentage from 0 to 100
     *
     * @param value Brightness value, from 0 to valueMax
     * @return The brightness percentage for the supplied value
     */
    float percentage(int value) const;

    /**
     * Retrieve a copy of the current brightness state
     *
     * @return A struct that contains the current brightness state.
     */
    const BrightnessInfo info() const;

    /**
     * Convert brightness step to raw brightness value
     *
     * @param step Brightness step, from 0 to steps
     * @return Brightness value that corresponds to the given step
     */
    int stepToValue(int step) const;

protected:
    /**
     * Calculate the optimal number of brightness steps.
     *
     * It should be based on three assumptions:
     * 1) The user generally expects to see equal brightness steps, and likes round percentage numbers.
     * 2) Actual brightness is rounded to whole percents before being displayed to the user.
     * 3) An assumption on a generally good number of brightness steps, which varies with implementations.
     *
     * This function does not depend on anything except the argument.
     *
     * @param valueMax the maximum brightness value for which we want to calculate the number of steps
     * @return the optimal maximum step number
     */
    virtual int calculateSteps(int valueMax) const = 0;

private:
    BrightnessInfo m_current;
};

}
