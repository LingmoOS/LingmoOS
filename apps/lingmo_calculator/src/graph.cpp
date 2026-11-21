// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "graph.h"
#include <algorithm>
#include <cmath>
namespace Calculator {
GraphResult sampleGraph(const std::string& expression, double cx, double cy, double hx, double hy,
                        AngleUnit angle, unsigned samples)
{
    GraphResult result;
    if (!std::isfinite(cx) || !std::isfinite(cy) || !std::isfinite(hx) || !std::isfinite(hy) || hx <= 0 || hy <= 0) {
        result.error = Error::Domain;
        return result;
    }
    samples = std::clamp(samples, 32u, 1600u);
    bool connected = false;
    GraphPoint previous{};
    for (unsigned i = 0; i <= samples; ++i) {
        const double x = cx - hx + 2 * hx * i / samples;
        const auto value = evaluate(expression, angle, x);
        if (value.error == Error::InvalidExpression || value.error == Error::TooLong) {
            result.error = value.error;
            result.segments.clear();
            return result;
        }
        const double y = static_cast<double>(value.value);
        if (value.error != Error::None || !std::isfinite(y) || std::abs(y - cy) > 4 * hy) {
            connected = false;
            continue;
        }
        if (connected) {
            const auto middle = evaluate(expression, angle, (previous.x + x) / 2);
            // Break near singularities rather than joining opposite sides of a pole.
            if (middle.error == Error::None && std::isfinite(middle.value)
                && std::abs(y - previous.y) < hy * 1.5
                && std::abs(middle.value - (previous.y + y) / 2) < hy * 0.16)
                result.segments.push_back({previous, {x, y}});
        }
        previous = {x, y};
        connected = true;
    }
    return result;
}
}
