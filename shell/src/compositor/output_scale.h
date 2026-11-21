// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string_view>

#include "base/geometry.h"

namespace Lingmo::Compositor {

class OutputScale
{
public:
    static constexpr uint32_t denominator = 120;

    static std::optional<OutputScale> fromNumerator(uint32_t value)
    {
        if (value < 120 || value > 480)
            return std::nullopt;
        return OutputScale(value);
    }

    // Digits with an optional fraction; one trailing newline is tolerated.
    static std::optional<OutputScale> parse(std::string_view text)
    {
        if (text.ends_with('\n'))
            text.remove_suffix(1);
        const auto digits = [](std::string_view part) {
            return !part.empty() && std::all_of(part.begin(), part.end(), [](char c) {
                return c >= '0' && c <= '9';
            });
        };
        const size_t point = text.find('.');
        if (!digits(text.substr(0, point))
            || (point != std::string_view::npos && !digits(text.substr(point + 1))))
            return std::nullopt;
        double value = 0;
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc() || end != text.data() + text.size() || !std::isfinite(value)
            || value < 1 || value > 4)
            return std::nullopt;
        return fromNumerator(static_cast<uint32_t>(std::lround(value * denominator)));
    }

    OutputScale() = default;

    uint32_t numerator() const { return m_numerator; }

    double value() const { return double(m_numerator) / denominator; }

    int integerScale() const { return (m_numerator + denominator - 1) / denominator; }

    base::Size logicalSize(const base::Size& native) const
    {
        // Cover the native target; the fractional last logical pixel is clipped.
        const auto extent = [this](int pixels) {
            return int((int64_t(pixels) * denominator + m_numerator - 1) / m_numerator);
        };
        return base::Size(extent(native.width()), extent(native.height()));
    }

    base::PointF toDevice(const base::PointF& logical) const { return logical * value(); }

    base::Rect deviceCoverage(const base::RectF& logical) const
    {
        return base::RectF(toDevice(logical.topLeft()), logical.size() * value()).toEnclosingRect();
    }

    bool operator==(const OutputScale& other) const { return m_numerator == other.m_numerator; }

private:
    explicit OutputScale(uint32_t value)
        : m_numerator(value)
    {
    }

    uint32_t m_numerator = denominator;
};

} // namespace Lingmo::Compositor
