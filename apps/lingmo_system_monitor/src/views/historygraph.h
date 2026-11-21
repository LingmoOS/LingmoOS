// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/view.h"

#include <cstdint>
#include <optional>
#include <vector>

// The last minute of a measurement: a filled curve over a grid, and an
// optional second series as a line.
class HistoryGraph : public Lingmo::ui::View
{
public:
    void setValues(std::vector<double> values);
    void setSecondaryValues(std::vector<double> values);
    // Milliseconds; the newest sample is at the right edge.
    void setTimestamps(std::vector<int64_t> timestamps);
    void setDuration(int64_t duration);
    void setColor(Lingmo::ui::Color color);
    void setSecondaryColor(Lingmo::ui::Color color);
    // Unset: no grid.
    void setGridColor(std::optional<Lingmo::ui::Color> color);
    // Positive: a fixed full scale; 0 picks the next power of two above the peak.
    void setMaximum(double maximum);
    // The smallest full scale an automatic maximum may pick.
    void setScaleFloor(double floor);
    void setCurved(bool curved);
    // Logical pixels.
    void setLineWidth(double width);
    double effectiveMaximum() const;

    struct Segment
    {
        Lingmo::ui::PointF control1;
        Lingmo::ui::PointF control2;
        Lingmo::ui::PointF end;
    };
    // The series as drawn in the rect: its first point, then cubic segments.
    static std::vector<Segment> curve(const std::vector<Lingmo::ui::PointF>& points, bool curved);

protected:
    void paint(Lingmo::ui::Painter& painter) override;

private:
    std::vector<Lingmo::ui::PointF> points(const std::vector<double>& values, const Lingmo::ui::RectF& area,
                                             bool filled) const;

    std::vector<double> m_values;
    std::vector<double> m_secondaryValues;
    std::vector<int64_t> m_timestamps;
    int64_t m_duration = 60000;
    Lingmo::ui::Color m_color;
    Lingmo::ui::Color m_secondaryColor;
    std::optional<Lingmo::ui::Color> m_gridColor;
    double m_maximum = 100;
    double m_scaleFloor = 1024;
    bool m_curved = false;
    double m_lineWidth = 1.5;
};
