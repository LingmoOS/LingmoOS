// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/battery/historygraph.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>

#include "base/i18n.h"
#include "base/locale.h"
#include "pages/battery/timeformat.h"
#include "ui/animation.h"
#include "ui/color_provider.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

// The curve and its marker reach this far past the plot, so a clip that
// draws the curve in leaves them whole.
static constexpr double kOverhang = 8;

static Color tint(Color color, double alpha)
{
    return color.withAlpha(alpha);
}

// Charging, fully charged, or held by the charge limit.
static bool pluggedIn(uint32_t state)
{
    return state == 1 || state == 4 || state == 5;
}

static std::string stateText(uint32_t state)
{
    switch (state) {
    case 1:
        return tr("Charging");
    case 4:
        return tr("Fully charged");
    case 5:
        return tr("Not charging");
    case 2:
    case 3:
    case 6:
        return tr("On battery");
    }
    return {};
}

// As JavaScript's toFixed: halves round away from zero.
static std::string fixed(double value, int decimals)
{
    const double scale = std::pow(10.0, decimals);
    const double rounded = std::floor(std::abs(value) * scale + 0.5) / scale;
    char text[32];
    snprintf(text, sizeof text, "%.*f", decimals, std::copysign(rounded, value));
    return text;
}

static int sign(double value)
{
    return (value > 0) - (value < 0);
}

// As JavaScript's Math.round, which rounds halves up, also below zero.
static double jsRound(double value)
{
    return std::floor(value + 0.5);
}

// Two dashes in the curve's colours, each with its name.
class HistoryGraph::Legend : public View
{
public:
    explicit Legend(HistoryGraph& graph)
        : m_graph(graph)
    {
        for (const char* text : {tr("On battery"), tr("Plugged in")})
            m_labels.push_back(addChild<Label>(text));
    }

    void setOpacityTarget(double target)
    {
        if (target == m_target)
            return;
        m_target = target;
        if (!window()) {
            setOpacity(target);
            return;
        }
        if (!m_fade) {
            m_fade = std::make_unique<Animation>(window());
            m_fade->setDuration(std::chrono::milliseconds(150));
            m_fade->setEasing(Animation::Easing::Linear);
            m_fade->onUpdate = [this](double value) { setOpacity(m_from + (m_target - m_from) * value); };
        }
        m_from = opacity();
        m_fade->start();
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        double x = 0;
        for (Label* label : m_labels) {
            label->setFontSize(m_graph.labelFontSize());
            label->setColor(theme.disabledText);
            const SizeF size = label->sizeHint();
            label->setGeometry({x + kDashWidth + kDashSpacing, m_graph.m_textBaseline - label->baselineOffset(),
                                size.width, size.height});
            x += kDashWidth + kDashSpacing + size.width + metrics::m;
        }
    }

    void paint(Painter& painter) override
    {
        // Level with the middle of the lowercase letters.
        const double y = m_graph.m_textBaseline - m_labels.front()->xHeight() / 2 - kDashHeight / 2;
        const Color colors[] = {m_graph.batteryColor(), m_graph.powerColor()};
        for (size_t i = 0; i < m_labels.size(); ++i) {
            const double x = m_labels[i]->geometry().x - kDashSpacing - kDashWidth;
            painter.drawRoundedRect({x, y, kDashWidth, kDashHeight}, {.radius = kDashHeight / 2}, colors[i]);
        }
    }

private:
    static constexpr double kDashWidth = 10;
    static constexpr double kDashHeight = 3;
    static constexpr double kDashSpacing = metrics::xs + metrics::xxs;

    HistoryGraph& m_graph;
    std::vector<Label*> m_labels;
    std::unique_ptr<Animation> m_fade;
    double m_from = 1;
    double m_target = 1;
};

// Follows the pointer over the plot, to read the curve off where it is.
class HistoryGraph::PlotArea : public View
{
public:
    explicit PlotArea(HistoryGraph& graph)
        : m_graph(graph)
    {
        setTracksHover(true);
    }

protected:
    void hoverMove(const PointerEvent& event) override { m_graph.setHoverX(event.position.x); }
    void hoverChanged() override
    {
        if (!isHovered())
            m_graph.setHoverX(std::nullopt);
    }

private:
    HistoryGraph& m_graph;
};

// The value and time under the pointer, in a pill over the legend.
class HistoryGraph::Readout : public View
{
public:
    explicit Readout(HistoryGraph& graph)
        : m_graph(graph)
    {
        m_value = addChild<Label>();
        m_value->setName("graph:readout:value");
        m_value->setWeight(600);
        m_time = addChild<Label>();
        m_time->setName("graph:readout:time");
    }

    void setTexts(std::string value, std::string time)
    {
        m_value->setText(std::move(value));
        m_time->setText(std::move(time));
    }

    double contentWidth() const { return m_value->sizeHint().width + kSpacing + m_time->sizeHint().width; }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const double fontSize = m_graph.labelFontSize();
        m_value->setFontSize(fontSize);
        m_value->setColor(theme.text);
        m_time->setFontSize(fontSize);
        m_time->setColor(theme.disabledText);
        const SizeF value = m_value->sizeHint();
        const SizeF time = m_time->sizeHint();
        double x = rounding::centeredStart(geometry().width, contentWidth());
        m_value->setGeometry({x, m_graph.m_textBaseline - m_value->baselineOffset(), value.width, value.height});
        x += value.width + kSpacing;
        m_time->setGeometry({x, m_graph.m_textBaseline - m_time->baselineOffset(), time.width, time.height});
    }

    void paint(Painter& painter) override
    {
        const RectF rect = localRect();
        painter.drawRoundedRect(rect, {.radius = rect.height / 2}, color(SettingsColor::BatteryChipBackground),
                                1 / painter.scale(), color(SettingsColor::BatteryChipBorder));
    }

private:
    static constexpr double kSpacing = metrics::xs + metrics::xxs;

    HistoryGraph& m_graph;
    Label* m_value;
    Label* m_time;
};

HistoryGraph::HistoryGraph()
{
    m_digits = addChild<Label>("0123456789");
    m_maximumLabel = addChild<Label>();
    m_timeLabel = addChild<Label>();
    m_dayLabel = addChild<Label>();
    for (Label* label : {m_digits, m_maximumLabel, m_timeLabel, m_dayLabel})
        label->setVisible(false);

    m_legend = addChild<Legend>(*this);
    m_legend->setName("graph:legend");
    m_plotArea = addChild<PlotArea>(*this);
    m_plotArea->setName("graph:plot");
    m_readout = addChild<Readout>(*this);
    m_readout->setName("graph:readout");
    m_readout->setVisible(false);
}

HistoryGraph::~HistoryGraph() = default;

void HistoryGraph::setPoints(std::vector<Point> points)
{
    m_points = std::move(points);
    dataChanged();
}

void HistoryGraph::setEnd(double end)
{
    if (end == m_end)
        return;
    m_end = end;
    dataChanged();
}

void HistoryGraph::setDuration(double duration)
{
    if (duration == m_duration)
        return;
    m_duration = duration;
    dataChanged();

    // A new range draws in again.
    if (m_revealAnimation)
        m_revealAnimation->stop();
    m_reveal = 0;
    if (m_curve.xs.size() > 1)
        startReveal();
}

void HistoryGraph::setValueAxis(double maximum, double step, std::string units, int decimals)
{
    m_maximum = maximum;
    m_step = step;
    m_units = std::move(units);
    m_decimals = decimals;
    dataChanged();
}

void HistoryGraph::dataChanged()
{
    m_curveDirty = true;
    requestLayout();
    requestPaint();
}

double HistoryGraph::labelFontSize() const
{
    return metrics::points(std::max(Theme::current().fontPointSize - 1, 7.0));
}

Color HistoryGraph::powerColor() const
{
    return accent::green;
}

Color HistoryGraph::batteryColor() const
{
    // A green accent would hide the plugged-in runs, so blue stands in for it.
    const Color highlight = Theme::current().highlight;
    return highlight == powerColor() ? accent::blue : highlight;
}

Label* HistoryGraph::addAxisLabel()
{
    Label* label = addChild<Label>();
    // Under the plot area, which takes the pointer.
    raiseChild(m_plotArea);
    raiseChild(m_readout);
    return label;
}

double HistoryGraph::xFor(double time) const
{
    return (time - (m_end - m_duration)) / m_duration * m_plot.width;
}

double HistoryGraph::yFor(double value) const
{
    return m_plot.height - std::min(value, m_maximum) / m_maximum * m_plot.height;
}

HistoryGraph::Ticks HistoryGraph::computeTicks() const
{
    static constexpr double steps[] = {600, 1800, 3600, 7200, 14400, 21600, 43200, 86400};
    const double timeWidth = m_timeLabel->sizeHint().width;
    const double dayWidth = m_dayLabel->sizeHint().width;
    double step = 0;

    // The smallest whole step that leaves seven ticks or fewer, with room
    // for their labels: a time, or a weekday where every tick starts a day.
    for (double candidate : steps) {
        const double count = m_duration / candidate;
        const double label = std::fmod(candidate, 86400) == 0 ? dayWidth : timeWidth;
        if (count <= 7 && count * (label + metrics::l) <= m_plot.width) {
            step = candidate;
            break;
        }
    }

    if (step == 0) {
        const double fit = std::floor(m_plot.width / (dayWidth + metrics::l));
        step = std::ceil(m_duration / std::max(1.0, std::min(7.0, fit)) / 86400) * 86400;
    }

    // On a whole local multiple of the step, e.g. midnight for every 4 hours.
    const double from = m_end - m_duration;
    const double offset = -double(timeformat::utcOffset(int64_t(std::floor(from))));
    const double first = std::ceil((from - offset) / step) * step + offset;

    return {step, first, m_end < first ? 0 : int(std::floor((m_end - first) / step)) + 1};
}

// A monotone cubic (Steffen's method): it bends smoothly but never
// overshoots a sample, so a full battery cannot read above 100%. UPower
// records a sample when the value changes, with the state that changed it,
// so the segment leading up to a sample takes that sample's state.
void HistoryGraph::buildCurve()
{
    m_curve = {};
    const double width = m_plot.width;
    const double height = m_plot.height;
    const double start = m_end - m_duration;
    Curve& c = m_curve;

    for (const Point& p : m_points) {
        if (p.time < start)
            continue;
        c.xs.push_back((p.time - start) / m_duration * width);
        c.ys.push_back(height - std::min(p.value, m_maximum) / m_maximum * height);
        c.states.push_back(p.state);
    }

    const size_t n = c.xs.size();
    if (n < 2) {
        c = {};
        return;
    }

    c.tangents.assign(n, 0);
    for (size_t i = 1; i < n - 1; ++i) {
        const double h0 = c.xs[i] - c.xs[i - 1];
        const double h1 = c.xs[i + 1] - c.xs[i];
        const double s0 = (c.ys[i] - c.ys[i - 1]) / h0;
        const double s1 = (c.ys[i + 1] - c.ys[i]) / h1;
        const double p = (s0 * h1 + s1 * h0) / (h0 + h1);
        const double t = (sign(s0) + sign(s1)) * std::min({std::abs(s0), std::abs(s1), 0.5 * std::abs(p)});
        c.tangents[i] = std::isfinite(t) ? t : 0;
    }

    const double firstSlope = (c.ys[1] - c.ys[0]) / (c.xs[1] - c.xs[0]);
    const double lastSlope = (c.ys[n - 1] - c.ys[n - 2]) / (c.xs[n - 1] - c.xs[n - 2]);
    c.tangents[0] = n > 2 ? (3 * firstSlope - c.tangents[1]) / 2 : firstSlope;
    c.tangents[n - 1] = n > 2 ? (3 * lastSlope - c.tangents[n - 2]) / 2 : lastSlope;

    // One subpath per run of segments in the same colour.
    size_t runStart = 0;
    std::vector<std::pair<PointF, std::array<PointF, 3>>> run;
    for (size_t i = 0; i < n - 1; ++i) {
        const double d = (c.xs[i + 1] - c.xs[i]) / 3;
        const bool power = pluggedIn(c.states[i + 1]);

        if (i == runStart)
            run.clear();
        run.push_back({{c.xs[i], c.ys[i]},
                       {PointF{c.xs[i] + d, c.ys[i] + d * c.tangents[i]},
                        PointF{c.xs[i + 1] - d, c.ys[i + 1] - d * c.tangents[i + 1]},
                        PointF{c.xs[i + 1], c.ys[i + 1]}}});

        if (i == n - 2 || pluggedIn(c.states[i + 2]) != power) {
            Path& line = power ? c.powerLine : c.batteryLine;
            Path& area = power ? c.powerArea : c.batteryArea;
            line.moveTo(run.front().first);
            area.moveTo(run.front().first);
            for (const auto& [from, segment] : run) {
                line.cubicTo(segment[0], segment[1], segment[2]);
                area.cubicTo(segment[0], segment[1], segment[2]);
            }
            area.lineTo({c.xs[i + 1], height});
            area.lineTo({c.xs[runStart], height});
            area.close();
            runStart = i + 1;
        }
    }
}

// The point on the curve at `x`, with the state of its segment.
std::optional<HistoryGraph::ProbePoint> HistoryGraph::pointAt(double x) const
{
    const Curve& c = m_curve;
    const size_t n = c.xs.size();
    if (n < 2)
        return std::nullopt;

    x = std::max(c.xs.front(), std::min(x, c.xs.back()));

    size_t i = 0;
    while (i < n - 2 && c.xs[i + 1] < x)
        ++i;

    // The segments are cubic Hermite curves with x linear in t.
    const double h = c.xs[i + 1] - c.xs[i];
    const double t = h > 0 ? (x - c.xs[i]) / h : 0;
    const double t2 = t * t;
    const double t3 = t2 * t;
    const double y = (2 * t3 - 3 * t2 + 1) * c.ys[i] + (t3 - 2 * t2 + t) * h * c.tangents[i]
                     + (-2 * t3 + 3 * t2) * c.ys[i + 1] + (t3 - t2) * h * c.tangents[i + 1];

    return ProbePoint{x, y, c.states[i + 1]};
}

void HistoryGraph::setHoverX(std::optional<double> x)
{
    if (x == m_hoverX)
        return;
    m_hoverX = x;
    requestLayout();
    requestPaint();
}

void HistoryGraph::startReveal()
{
    if (!window()) {
        m_reveal = 1;
        return;
    }
    if (!m_revealAnimation) {
        m_revealAnimation = std::make_unique<Animation>(window());
        m_revealAnimation->setDuration(std::chrono::milliseconds(700));
        m_revealAnimation->setEasing(Animation::Easing::OutCubic);
        m_revealAnimation->onUpdate = [this](double value) {
            m_reveal = value;
            requestLayout();
            requestPaint();
        };
    }
    m_revealAnimation->start();
}

void HistoryGraph::layout()
{
    const Theme& theme = Theme::current();
    const double fontSize = labelFontSize();
    const double scale = window() ? window()->scale() : 1;
    const double width = geometry().width;
    const double height = geometry().height;

    m_maximumLabel->setText(fixed(m_maximum, 0) + m_units);
    // The widest time label, e.g. "12:00 pm", and a weekday.
    const base::Locale& locale = base::Locale::system();
    m_timeLabel->setText(locale.toString(base::ClockTime{12, 0}, base::Locale::FormatType::Short));
    m_dayLabel->setText(locale.dayName(3, base::Locale::FormatType::Short));
    for (Label* label : {m_digits, m_maximumLabel, m_timeLabel, m_dayLabel})
        label->setFontSize(fontSize);

    // As FontMetrics.height and the digits' TextMetrics.tightBoundingRect.
    const double lineHeight = std::ceil(m_digits->ascent() + m_digits->descent());
    const double digitHeight = m_digits->tightBoundingRect().height;

    // Text in the header sits on one baseline that centres the digits.
    m_headerHeight = lineHeight + metrics::s;
    m_textBaseline = jsRound((m_headerHeight + digitHeight) / 2 * scale) / scale;

    // Half a label above the top line, so its label is not cut off, and one
    // label line below for the time.
    const double plotY = m_headerHeight + metrics::s + std::ceil(lineHeight / 2);
    const RectF plot{0, plotY, width - std::ceil(m_maximumLabel->sizeHint().width) - metrics::m,
                     height - plotY - lineHeight - metrics::s};
    if (plot.width != m_plot.width || plot.height != m_plot.height)
        m_curveDirty = true;
    m_plot = plot;

    m_legend->setGeometry({0, 0, width, m_headerHeight});
    m_plotArea->setGeometry(m_plot);

    // The value axis: the digits, not the line box, centred on each line.
    const int lines = int(std::floor(m_maximum / m_step)) + 1;
    while (int(m_valueLabels.size()) < lines)
        m_valueLabels.push_back(addAxisLabel());
    for (size_t i = 0; i < m_valueLabels.size(); ++i) {
        Label* label = m_valueLabels[i];
        label->setVisible(int(i) < lines);
        if (int(i) >= lines)
            continue;
        const double value = double(i) * m_step;
        const double lineY = jsRound((yFor(value) - 0.5 / scale) * scale) / scale;
        label->setText(fixed(value, 0) + m_units);
        label->setFontSize(fontSize);
        label->setColor(theme.disabledText);
        const SizeF size = label->sizeHint();
        label->setGeometry({m_plot.x + m_plot.width + metrics::m,
                            m_plot.y + lineY + 0.5 / scale + digitHeight / 2 - label->baselineOffset(), size.width,
                            size.height});
    }

    // The time axis: a time, or the weekday at the start of a day.
    m_ticks = computeTicks();
    while (int(m_tickLabels.size()) < m_ticks.count)
        m_tickLabels.push_back(addAxisLabel());
    for (size_t i = 0; i < m_tickLabels.size(); ++i) {
        Label* label = m_tickLabels[i];
        label->setVisible(int(i) < m_ticks.count);
        if (int(i) >= m_ticks.count)
            continue;
        const double time = m_ticks.first + double(i) * m_ticks.step;
        const int64_t secs = int64_t(std::floor(time));
        const bool dayStart = timeformat::isStartOfDay(secs);
        label->setText(dayStart ? timeformat::shortDayName(secs) : timeformat::shortTime(secs));
        label->setFontSize(fontSize);
        label->setColor(dayStart ? theme.text : theme.disabledText);
        const SizeF size = label->sizeHint();
        // Centred under the tick, but kept inside the graph at either end.
        const double tickX = xFor(time);
        const double x = std::max(-tickX, std::min(width - m_plot.x - tickX - size.width, -size.width / 2));
        label->setGeometry({m_plot.x + tickX + x, m_plot.bottom() + metrics::s, size.width, size.height});
    }

    if (m_curveDirty) {
        m_curveDirty = false;
        buildCurve();
        // Draws the curve in from the left once, when the first samples arrive.
        const bool running = m_revealAnimation && m_revealAnimation->isRunning();
        if (m_curve.xs.size() > 1 && m_reveal == 0 && !running)
            startReveal();
    }

    // The reading under the pointer, in the legend's place.
    m_probe = m_hoverX ? pointAt(*m_hoverX) : std::nullopt;
    const bool reading = m_probe && m_reveal >= 1;
    m_legend->setOpacityTarget(reading ? 0 : 1);
    m_readout->setVisible(reading);
    if (reading) {
        const double value = (m_plot.height - m_probe->y) / m_plot.height * m_maximum;
        const double start = m_end - m_duration;
        const int64_t secs = int64_t(std::floor(start + m_probe->x / m_plot.width * m_duration));
        std::string time = timeformat::shortTime(secs);
        if (m_duration > 86400)
            // TRANSLATORS: a weekday and a time in the graph's readout, such as "Mon 9:41 AM".
            time = format(tr("{day} {time}"), {{"day", timeformat::shortDayName(secs)}, {"time", time}});
        const std::string state = stateText(m_probe->state);
        std::string detail = time;
        if (!state.empty())
            // TRANSLATORS: the battery state and the time in the graph's readout, such as "Charging · 9:41 AM".
            detail = format(tr("{state} · {time}"), {{"state", state}, {"time", time}});
        m_readout->setTexts(fixed(value, m_decimals) + m_units, detail);
        const double readoutWidth = m_readout->contentWidth() + metrics::s * 2;
        const double x = std::max(0.0, std::min(width - readoutWidth, m_plot.x + m_probe->x - readoutWidth / 2));
        m_readout->setGeometry({x, 0, readoutWidth, m_headerHeight});
    }
}

void HistoryGraph::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const double scale = painter.scale();
    const double hairline = 1 / scale;
    const Color gridColor = color(SettingsColor::GraphGrid);
    const Color baselineColor = color(SettingsColor::GraphBaseline);

    painter.save();
    painter.translate({m_plot.x, m_plot.y});

    // On the device pixel grid, so the labels centre on the lines as drawn.
    const int lines = int(std::floor(m_maximum / m_step)) + 1;
    for (int i = 0; i < lines; ++i) {
        const double y = jsRound((yFor(double(i) * m_step) - hairline / 2) * scale) / scale;
        painter.drawRoundedRect({0, y, m_plot.width, hairline}, {}, i == 0 ? baselineColor : gridColor);
    }

    // Only a new day gets a line; the hours are read off the labels.
    for (int i = 0; i < m_ticks.count; ++i) {
        const double time = m_ticks.first + double(i) * m_ticks.step;
        if (timeformat::isStartOfDay(int64_t(std::floor(time))))
            painter.drawRoundedRect({xFor(time) - hairline / 2, 0, hairline, m_plot.height}, {}, gridColor);
    }

    if (m_curve.xs.size() > 1) {
        const double areaOpacity = theme.darkMode ? 0.3 : 0.2;
        const Color battery = batteryColor();
        const Color power = powerColor();

        painter.save();
        // Clips the curve only sideways, and only while it is drawn in.
        if (m_reveal < 1)
            painter.clip({-kOverhang, -kOverhang, (m_plot.width + kOverhang * 2) * m_reveal,
                          m_plot.height + kOverhang * 2});
        const RectF area{0, 0, m_plot.width, m_plot.height};
        painter.fillPath(m_curve.batteryArea,
                         LinearGradient::vertical(area, tint(battery, areaOpacity), tint(battery, 0)));
        painter.fillPath(m_curve.powerArea, LinearGradient::vertical(area, tint(power, areaOpacity), tint(power, 0)));
        painter.strokePath(m_curve.batteryLine, battery, 2);
        painter.strokePath(m_curve.powerLine, power, 2);
        painter.restore();

        if (m_probe && m_reveal >= 1) {
            const Color probeColor = color(SettingsColor::GraphProbe);
            const double top = m_headerHeight - m_plot.y;
            painter.drawRoundedRect({m_probe->x - hairline / 2, top, hairline, m_plot.height - top}, {}, probeColor);

            // A soft halo, then a ring in the card's colour, so the dot stays
            // clear of the curve and the grid lines it sits on.
            const Color color = pluggedIn(m_probe->state) ? power : battery;
            const auto circle = [this](double diameter) {
                return RectF{m_probe->x - diameter / 2, m_probe->y - diameter / 2, diameter, diameter};
            };
            painter.drawSmoothRectangle(circle(18), 9, tint(color, 0.18));
            painter.drawSmoothRectangle(circle(12), 6, theme.secondBackground);
            painter.drawSmoothRectangle(circle(8), 4, color);
        }
    }

    painter.restore();
}

} // namespace settings
