/*
 *   SPDX-FileCopyrightText: 2014 Nikita Skovoroda <chalkerx@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "powerdevilkeyboardbrightnesslogic.h"
#include "powerdevil_debug.h"
#include <QDebug>

namespace PowerDevil
{
int KeyboardBrightnessLogic::calculateSteps(int maxValue) const
{
    // We assume that a generally good number of steps for keyboard brightness is about 5.

    if (maxValue <= 7) {
        // Too few steps, return the number of actual steps.
        return maxValue;
    }

    if (maxValue % 5 == 0 || maxValue >= 80) {
        // When there are more than 80 actual steps, 20%, 40%, 60%, and 80% are always displayed nicely.
        return 5;
    }

    if (maxValue % 4 == 0)
        return 4;

    if (maxValue % 6 == 0)
        return 6;

    if (maxValue % 3 == 0)
        return 3;

    // 29 different maxValue values between 11 and 79 are left at this point.
    // qCDebug(POWERDEVIL) << "maxValue" << maxValue;

    // Give up and return 5, there is nothing much we can do here.
    return 5;
}

}
