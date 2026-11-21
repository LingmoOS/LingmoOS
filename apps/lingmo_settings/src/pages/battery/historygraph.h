// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ui/path.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Animation;
class Label;
}

namespace settings {

// Plots history samples over the `duration` seconds up to `end` as a smooth
// curve, coloured by whether the computer was plugged in, with a legend, a
// value axis on the right and a time axis below. The pointer reads the curve
// off: a marker on it, and the value and time in place of the legend.
class HistoryGraph : public Lingmo::ui::View
{
public:
    struct Point
    {
        // Unix seconds.
        double time = 0;
        double value = 0;
        // UPower's device state when the sample was taken.
        uint32_t state = 0;
    };

    HistoryGraph();
    ~HistoryGraph() override;

    void setPoints(std::vector<Point> points);
    // Unix seconds at the right edge.
    void setEnd(double end);
    // Seconds; a change draws the curve in again.
    void setDuration(double duration);
    void setValueAxis(double maximum, double step, std::string units, int decimals);

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    class Legend;
    class PlotArea;
    class Readout;

    struct Curve
    {
        std::vector<double> xs;
        std::vector<double> ys;
        std::vector<double> tangents;
        std::vector<uint32_t> states;
        Lingmo::ui::Path batteryLine;
        Lingmo::ui::Path powerLine;
        Lingmo::ui::Path batteryArea;
        Lingmo::ui::Path powerArea;
    };

    struct ProbePoint
    {
        double x = 0;
        double y = 0;
        uint32_t state = 0;
    };

    struct Ticks
    {
        double step = 0;
        double first = 0;
        int count = 0;
    };

    void dataChanged();
    void buildCurve();
    Ticks computeTicks() const;
    std::optional<ProbePoint> pointAt(double x) const;
    double xFor(double time) const;
    double yFor(double value) const;
    void startReveal();
    void setHoverX(std::optional<double> x);
    Lingmo::ui::Color batteryColor() const;
    Lingmo::ui::Color powerColor() const;
    double labelFontSize() const;
    Lingmo::ui::Label* addAxisLabel();

    std::vector<Point> m_points;
    double m_end = 0;
    double m_duration = 86400;
    double m_maximum = 100;
    double m_step = 25;
    std::string m_units = "%";
    int m_decimals = 0;

    Curve m_curve;
    bool m_curveDirty = true;
    double m_reveal = 0;
    std::unique_ptr<Lingmo::ui::Animation> m_revealAnimation;

    // Where the plot sits in the graph, from the last layout.
    Lingmo::ui::RectF m_plot;
    double m_headerHeight = 0;
    double m_textBaseline = 0;
    Ticks m_ticks;

    std::optional<double> m_hoverX;
    std::optional<ProbePoint> m_probe;

    Legend* m_legend;
    PlotArea* m_plotArea;
    Readout* m_readout;
    std::vector<Lingmo::ui::Label*> m_valueLabels;
    std::vector<Lingmo::ui::Label*> m_tickLabels;
    // For measuring only, never shown.
    Lingmo::ui::Label* m_digits;
    Lingmo::ui::Label* m_maximumLabel;
    Lingmo::ui::Label* m_timeLabel;
    Lingmo::ui::Label* m_dayLabel;
};

} // namespace settings
