#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Cadence {

template<typename T, typename Enable = void>
struct Interpolator;

template<typename T>
struct Interpolator<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static T mix(T from, T to, double progress)
    {
        if (!std::isfinite(progress))
            return from;
        const long double value = static_cast<long double>(from)
                + (static_cast<long double>(to) - static_cast<long double>(from)) * progress;
        if constexpr (std::is_integral_v<T>) {
            const long double rounded = std::round(value);
            return static_cast<T>(std::clamp(rounded, static_cast<long double>(std::numeric_limits<T>::lowest()),
                                             static_cast<long double>(std::numeric_limits<T>::max())));
        } else {
            return static_cast<T>(value);
        }
    }
};

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    bool operator==(const Color &) const = default;
};

struct Point {
    double x = 0;
    double y = 0;

    bool operator==(const Point &) const = default;
};

struct Size {
    double width = 0;
    double height = 0;

    bool operator==(const Size &) const = default;
};

struct Rect {
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;

    bool operator==(const Rect &) const = default;
};

template<>
struct Interpolator<Color> {
    static Color mix(Color from, Color to, double progress)
    {
        const auto channel = [progress](uint8_t a, uint8_t b) {
            if (!std::isfinite(progress))
                return a;
            const double value = a + (static_cast<double>(b) - a) * progress;
            return static_cast<uint8_t>(std::lround(std::clamp(value, 0.0, 255.0)));
        };
        return {channel(from.r, to.r), channel(from.g, to.g), channel(from.b, to.b), channel(from.a, to.a)};
    }
};

template<>
struct Interpolator<Point> {
    static Point mix(Point from, Point to, double progress)
    {
        return {Interpolator<double>::mix(from.x, to.x, progress),
                Interpolator<double>::mix(from.y, to.y, progress)};
    }
};

template<>
struct Interpolator<Size> {
    static Size mix(Size from, Size to, double progress)
    {
        return {Interpolator<double>::mix(from.width, to.width, progress),
                Interpolator<double>::mix(from.height, to.height, progress)};
    }
};

template<>
struct Interpolator<Rect> {
    static Rect mix(Rect from, Rect to, double progress)
    {
        return {Interpolator<double>::mix(from.x, to.x, progress),
                Interpolator<double>::mix(from.y, to.y, progress),
                Interpolator<double>::mix(from.width, to.width, progress),
                Interpolator<double>::mix(from.height, to.height, progress)};
    }
};

} // namespace Cadence
