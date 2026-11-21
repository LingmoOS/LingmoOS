// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/label.h"

// What FishUI.AlignedLabel places text by, in logical pixels at the label's scale.
struct TextMetrics
{
    // The top of the line to the baseline.
    double ascent = 0;
    double lineHeight = 0;
    // The height of the capitals.
    double capHeight = 0;
};

TextMetrics textMetrics(const Lingmo::ui::Label* label);

// A logical length on the device pixel grid of the view's window.
double snapToDevice(const Lingmo::ui::View* view, double value);
