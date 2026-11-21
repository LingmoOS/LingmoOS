// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "calculator.h"
namespace Calculator {
struct GraphPoint { double x; double y; };
struct GraphSegment { GraphPoint from; GraphPoint to; };
struct GraphResult { std::vector<GraphSegment> segments; Error error = Error::None; };
GraphResult sampleGraph(const std::string& expression, double centerX, double centerY,
                        double halfWidth, double halfHeight, AngleUnit angle, unsigned samples = 800);
}
