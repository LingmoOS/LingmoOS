// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "historygraph.h"

#include "ui/painter.h"
#include "ui/path.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;

void HistoryGraph::setValues(std::vector<double> values)
{
    m_values = std::move(values);
    requestPaint();
}

void HistoryGraph::setSecondaryValues(std::vector<double> values)
{
    m_secondaryValues = std::move(values);
    requestPaint();
}

void HistoryGraph::setTimestamps(std::vector<int64_t> timestamps)
{
    m_timestamps = std::move(timestamps);
    requestPaint();
}

void HistoryGraph::setDuration(int64_t duration)
{
    if (duration <= 0 || duration == m_duration)
        return;
    m_duration = duration;
    requestPaint();
}

void HistoryGraph::setColor(Color color)
{
    m_color = color;
    requestPaint();
}

void HistoryGraph::setSecondaryColor(Color color)
{
    m_secondaryColor = color;
    requestPaint();
}

void HistoryGraph::setGridColor(std::optional<Color> color)
{
    m_gridColor = color;
    requestPaint();
}

void HistoryGraph::setMaximum(double maximum)
{
    m_maximum = maximum;
    requestPaint();
}

void HistoryGraph::setScaleFloor(double floor)
{
    m_scaleFloor = floor;
    requestPaint();
}

void HistoryGraph::setCurved(bool curved)
{
    m_curved = curved;
    requestPaint();
}

void HistoryGraph::setLineWidth(double width)
{
    m_lineWidth = width;
    requestPaint();
}

double HistoryGraph::effectiveMaximum() const
{
    if (m_maximum > 0)
        return m_maximum;
    double peak = m_scaleFloor;
    for (const auto* series : {&m_values, &m_secondaryValues}) {
        for (const double value : *series)
            peak = std::max(peak, value);
    }
    return std::pow(2.0, std::ceil(std::log2(peak)));
}

// Monotone cubic interpolation: the curve passes through every sample and never overshoots
// between them, so it shows no peak or dip that was not measured.
std::vector<HistoryGraph::Segment> HistoryGraph::curve(const std::vector<PointF>& points, bool curved)
{
    std::vector<Segment> segments;
    const size_t count = points.size();
    if (!curved || count < 3) {
        for (size_t i = 1; i < count; ++i)
            segments.push_back({points[i - 1], points[i], points[i]});
        return segments;
    }

    std::vector<double> slopes(count - 1);
    for (size_t i = 0; i < count - 1; ++i) {
        const double dx = points[i + 1].x - points[i].x;
        slopes[i] = dx > 0 ? (points[i + 1].y - points[i].y) / dx : 0;
    }

    std::vector<double> tangents(count);
    tangents[0] = slopes[0];
    tangents[count - 1] = slopes[count - 2];
    for (size_t i = 1; i < count - 1; ++i) {
        const double before = slopes[i - 1];
        const double after = slopes[i];
        if (before * after <= 0) {
            tangents[i] = 0;
            continue;
        }
        const double h0 = points[i].x - points[i - 1].x;
        const double h1 = points[i + 1].x - points[i].x;
        const double average = (before * h1 + after * h0) / (h0 + h1);
        const double sign = before > 0 ? 1 : -1;
        tangents[i] = sign * std::min(std::min(std::abs(before), std::abs(after)) * 2, std::abs(average));
    }

    for (size_t i = 0; i < count - 1; ++i) {
        const double third = (points[i + 1].x - points[i].x) / 3;
        segments.push_back({{points[i].x + third, points[i].y + tangents[i] * third},
                            {points[i + 1].x - third, points[i + 1].y - tangents[i + 1] * third},
                            points[i + 1]});
    }
    return segments;
}

std::vector<PointF> HistoryGraph::points(const std::vector<double>& values, const RectF& area, bool filled) const
{
    std::vector<PointF> result;
    if (values.empty() && !filled)
        return result;

    // History before the first sample is drawn as idle: flat along the bottom, rising to the
    // first sample over one sampling interval. An empty primary series is just that baseline.
    const double scale = effectiveMaximum();
    const bool timed = m_timestamps.size() == values.size();
    for (size_t i = 0; i < values.size(); ++i) {
        const int64_t age = timed ? m_timestamps.back() - m_timestamps[i] : int64_t(values.size() - 1 - i) * 1000;
        const double x = area.right() - area.width * double(age) / double(m_duration);
        const double y = area.bottom() - area.height * std::clamp(values[i] / scale, 0.0, 1.0);
        result.push_back({x, y});
    }
    if (result.empty()) {
        result = {{area.x, area.bottom()}, {area.right(), area.bottom()}};
    } else if (result.front().x > area.x) {
        const double step = result.size() > 1 ? result[1].x - result[0].x
                                              : area.width * 1000 / double(m_duration);
        const double rampStart = std::max(area.x, result.front().x - step);
        if (rampStart > area.x)
            result.insert(result.begin(), {rampStart, area.bottom()});
        result.insert(result.begin(), {area.x, area.bottom()});
    }
    return result;
}

void HistoryGraph::paint(Painter& painter)
{
    const RectF area{1, 2, std::max(0.0, geometry().width - 2), std::max(0.0, geometry().height - 4)};
    if (area.width <= 0 || area.height <= 0)
        return;
    if (m_gridColor) {
        for (int i = 0; i <= 4; ++i) {
            const double y = area.y + area.height * i / 4;
            Path line;
            line.moveTo({area.x, y});
            line.lineTo({area.right(), y});
            painter.strokePath(line, *m_gridColor, 1, LineCap::Square, LineJoin::Bevel);
        }
    }

    painter.save();
    painter.clip(localRect());
    const auto draw = [&](const std::vector<double>& values, Color color, bool filled) {
        const std::vector<PointF> points = this->points(values, area, filled);
        if (points.empty())
            return;
        Path line;
        line.moveTo(points.front());
        for (const Segment& segment : curve(points, m_curved))
            line.cubicTo(segment.control1, segment.control2, segment.end);
        if (filled) {
            Path fill = line;
            fill.lineTo({points.back().x, area.bottom()});
            fill.lineTo({points.front().x, area.bottom()});
            fill.close();
            painter.fillPath(fill, LinearGradient::vertical(area, color.withAlpha(0.18), color.withAlpha(0.02)));
        }
        painter.strokePath(line, color, m_lineWidth, LineCap::Round, LineJoin::Round);
    };
    draw(m_values, m_color, true);
    draw(m_secondaryValues, m_secondaryColor, false);
    painter.restore();
}
