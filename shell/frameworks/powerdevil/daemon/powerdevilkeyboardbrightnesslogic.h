/*
 *   SPDX-FileCopyrightText: 2014 Nikita Skovoroda <chalkerx@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "powerdevilbrightnesslogic.h"

namespace PowerDevil
{
class KeyboardBrightnessLogic : public BrightnessLogic
{
protected:
    int calculateSteps(int valueMax) const override;
};

}
