// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textmetrics.h"

#include "ui/window.h"

#include <cmath>

using namespace Lingmo::ui;

TextMetrics textMetrics(const Label* label)
{
    return {label->baselineOffset(), label->sizeHint().height, label->capHeight()};
}

double snapToDevice(const View* view, double value)
{
    const Window* window = view->window();
    const double scale = window ? window->scale() : 1.0;
    return std::round(value * scale) / scale;
}
