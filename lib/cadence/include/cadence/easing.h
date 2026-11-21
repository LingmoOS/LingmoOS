#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Cadence {

enum class Easing {
    Linear,
    InQuad,
    OutQuad,
    InOutQuad,
    OutCubic,
    InOutCubic,
    OutQuint,
    OutSine,
    InOutSine,
    OutBack,
};

inline double ease(Easing curve, double progress)
{
    if (std::isnan(progress))
        return 0;
    const double t = std::clamp(progress, 0.0, 1.0);
    switch (curve) {
    case Easing::Linear:
        return t;
    case Easing::InQuad:
        return t * t;
    case Easing::OutQuad:
        return 1 - (1 - t) * (1 - t);
    case Easing::InOutQuad:
        return t < 0.5 ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
    case Easing::OutCubic:
        return 1 - std::pow(1 - t, 3);
    case Easing::InOutCubic:
        return t < 0.5 ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
    case Easing::OutQuint:
        return 1 - std::pow(1 - t, 5);
    case Easing::OutSine:
        return std::sin(t * std::numbers::pi / 2);
    case Easing::InOutSine:
        return -(std::cos(std::numbers::pi * t) - 1) / 2;
    case Easing::OutBack: {
        constexpr double overshoot = 1.70158;
        const double x = t - 1;
        return 1 + (overshoot + 1) * x * x * x + overshoot * x * x;
    }
    }
    return t;
}

} // namespace Cadence
