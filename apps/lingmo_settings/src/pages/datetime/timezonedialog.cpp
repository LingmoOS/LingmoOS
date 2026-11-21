// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/datetime/timezonedialog.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "ui/animation.h"
#include "ui/color_provider.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/scroll_view.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/window_header.h"
#include "widgets/card.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "window/settingswindow.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kWidth = 900;
static constexpr double kHeight = 640;
static constexpr double kHeaderHeight = 48;
static constexpr double kSearchWidth = 280;
static constexpr double kSearchHeight = 30;
static constexpr double kResultsWidth = 360;
static constexpr double kResultsMaximumHeight = 360;
// As far in as a menu's rows, with a menu's corners.
static constexpr double kResultsPadding = 6;
static constexpr double kResultHeight = metrics::rowHeight + metrics::xs;
// The world map's own proportions.
static constexpr double kMapAspect = 978.0 / 500.0;
static const double kSmallFont = metrics::points(8);

static Path circle(PointF centre, double radius)
{
    // Four cubic quarters, as close to a circle as the eye can tell.
    const double k = 0.5522847498 * radius;
    Path path;
    path.moveTo({centre.x + radius, centre.y});
    path.cubicTo({centre.x + radius, centre.y + k}, {centre.x + k, centre.y + radius}, {centre.x, centre.y + radius});
    path.cubicTo({centre.x - k, centre.y + radius}, {centre.x - radius, centre.y + k}, {centre.x - radius, centre.y});
    path.cubicTo({centre.x - radius, centre.y - k}, {centre.x - k, centre.y - radius}, {centre.x, centre.y - radius});
    path.cubicTo({centre.x + k, centre.y - radius}, {centre.x + radius, centre.y - k}, {centre.x + radius, centre.y});
    path.close();
    return path;
}

static std::string joined(const std::vector<std::string>& parts, const std::string& separator)
{
    std::string text;
    for (const std::string& part : parts) {
        if (part.empty())
            continue;
        if (!text.empty())
            text += separator;
        text += part;
    }
    return text;
}

static double outCubic(double t)
{
    const double inverse = 1 - t;
    return 1 - inverse * inverse * inverse;
}

namespace {

// A label whose colour and size follow the theme, set by a function.
class StyledLabel : public Label
{
public:
    explicit StyledLabel(std::string text = {})
        : Label(std::move(text))
    {
    }

    std::function<void(StyledLabel&)> style;

protected:
    void layout() override
    {
        if (style)
            style(*this);
        Label::layout();
    }
};

// The search box: FishUI's field in a pill, its fill see-through so it sits
// lightly on the window; Up and Down move through the results, Return picks
// and Escape closes them.
// The field inside the search box, which draws the box's focus ring.
class SearchBoxField : public TextField
{
protected:
    void focusIn(FocusReason reason) override
    {
        TextField::focusIn(reason);
        parent()->requestPaint();
    }
    void focusOut() override
    {
        TextField::focusOut();
        parent()->requestPaint();
    }
};

class SearchBox : public View
{
public:
    SearchBox()
    {
        field = addChild<SearchBoxField>();
        field->setPlaceholder(tr("Search city or country"));
        field->setFrameVisible(false);
        field->setTextMargins(metrics::m + 14 + metrics::s, metrics::m);
        m_glyph = addChild<Icon>("search", 14);
    }

    TextField* field;
    std::function<bool(uint32_t key)> onKey;

    RectF visualRect() const override { return localRect().adjusted(-2, -2, 2, 2); }

protected:
    void layout() override
    {
        m_glyph->setColor(Theme::current().disabledText);
        const double height = geometry().height;
        m_glyph->setGeometry({metrics::m, rounding::centeredStart(height, 14), 14, 14});
        field->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const RectF rect = localRect();
        const Color fill = theme.alternateBackground.withAlpha(0.6);
        const bool focused = field->hasFocus();
        painter.fillRoundedRect(rect, rect.height / 2, fill);
        painter.strokeRoundedRect(rect, rect.height / 2, focused ? theme.highlight : fill, painter.borderWidth(1));
        if (focused) {
            // The field's radius; the inset grows it into a concentric ring.
            const RectF ring = rect.adjusted(-2, -2, 2, 2);
            painter.strokeRoundedRect(ring, ring.height / 2, theme.highlight.withAlpha(0.25), painter.borderWidth(2));
        }
    }

    bool keyPress(const KeyEvent& event) override { return onKey && onKey(event.key); }

private:
    Icon* m_glyph;
};

// One search result: the city and its country, its clock, and its day and
// difference.
class ResultRow : public View
{
public:
    ResultRow(const TimeZoneMap::Info& info, int index)
        : index(index)
    {
        m_row = addChild<RowLayout>();
        m_row->setSpacing(metrics::rowSpacing);
        ColumnLayout* left = m_row->addChild<ColumnLayout>();
        left->setSpacing(0);
        left->setLayoutHints({.fillWidth = true});
        m_name = left->addChild<StyledLabel>(info.name);
        m_name->setLayoutHints({.fillWidth = true});
        m_region = left->addChild<StyledLabel>(info.region);
        m_region->setFontSize(kSmallFont);
        m_region->setLayoutHints({.fillWidth = true});
        ColumnLayout* right = m_row->addChild<ColumnLayout>();
        right->setSpacing(0);
        m_time = right->addChild<StyledLabel>(info.time);
        m_time->setFontFeatures({{"tnum", 1}});
        m_time->setLayoutHints({.horizontalAlignment = Alignment::End});
        m_detail = right->addChild<StyledLabel>(joined({info.day, info.difference}, " · "));
        m_detail->setFontSize(kSmallFont);
        m_detail->setLayoutHints({.horizontalAlignment = Alignment::End});
        for (StyledLabel* label : {m_name, m_region, m_time, m_detail}) {
            label->style = [this](StyledLabel& label) {
                const Theme& theme = Theme::current();
                const bool primary = &label == m_name || &label == m_time;
                label.setColor(current ? theme.highlightedText : primary ? theme.text : theme.disabledText);
                label.setOpacity(current && !primary ? 0.8 : 1);
            };
        }
    }

    const int index;
    bool current = false;
    std::function<void(int index)> onPicked;

    void setCurrent(bool isCurrent)
    {
        if (isCurrent == current)
            return;
        current = isCurrent;
        // The labels take their colours from it as they lay out.
        for (StyledLabel* label : {m_name, m_region, m_time, m_detail})
            label->requestLayout();
        requestPaint();
    }

    SizeF sizeHint() const override { return {0, kResultHeight}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        m_row->setGeometry(localRect().adjusted(metrics::m, 0, -metrics::m, 0));
    }

    void paint(Painter& painter) override
    {
        if (!current)
            return;
        // Concentric with the popup's corners.
        const Theme& theme = Theme::current();
        painter.fillRoundedRect(localRect(), theme.windowRadius - kResultsPadding, theme.highlight, panelCorners());
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left && localRect().contains(event.position) && onPicked)
            onPicked(index);
    }

private:
    RowLayout* m_row;
    StyledLabel* m_name;
    StyledLabel* m_region;
    StyledLabel* m_time;
    StyledLabel* m_detail;
};

// The results under the search box, wider than it: each result carries a
// time and a day.
class ResultsPopup : public View
{
public:
    ResultsPopup()
    {
        m_scroll = addChild<ScrollView>();
        m_scroll->setElastic(false);
        m_rows = m_scroll->setContent<Column>();
        m_empty = addChild<StyledLabel>(tr("No matching time zones"));
        m_empty->style = [](StyledLabel& label) { label.setColor(Theme::current().disabledText); };
    }

    std::function<void(int index)> onPicked;

    int count() const { return int(m_results.size()); }
    int currentIndex() const { return m_current; }
    const std::vector<TimeZoneMap::Info>& results() const { return m_results; }

    void setResults(std::vector<TimeZoneMap::Info> results)
    {
        m_results = std::move(results);
        while (!m_rows->children().empty())
            m_rows->takeChild(m_rows->children().front().get());
        for (size_t i = 0; i < m_results.size(); ++i) {
            ResultRow* row = m_rows->addChild<ResultRow>(m_results[i], int(i));
            row->setLayoutHints({.fillWidth = true});
            row->onPicked = [this](int index) {
                if (onPicked)
                    onPicked(index);
            };
        }
        m_scroll->setScrollPosition({0, 0});
        setCurrentIndex(m_results.empty() ? -1 : 0);
        requestLayout();
    }

    void setCurrentIndex(int index)
    {
        m_current = index;
        for (const auto& child : m_rows->children()) {
            auto* row = static_cast<ResultRow*>(child.get());
            row->setCurrent(row->index == index);
        }
        if (index >= 0)
            m_scroll->ensureVisible({0, index * kResultHeight, 1, kResultHeight});
    }

    double popupHeight() const
    {
        if (m_results.empty())
            return m_empty->sizeHint().height + metrics::l * 2;
        return std::min(kResultHeight * double(m_results.size()) + kResultsPadding * 2, kResultsMaximumHeight);
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const RectF inner = localRect().adjusted(kResultsPadding, kResultsPadding, -kResultsPadding, -kResultsPadding);
        m_scroll->setGeometry(inner);
        m_scroll->setVisible(!m_results.empty());
        const SizeF empty = m_empty->sizeHint();
        m_empty->setGeometry({rounding::centeredStart(geometry().width, empty.width), rounding::centeredStart(geometry().height, empty.height),
                              empty.width, empty.height});
        m_empty->setVisible(m_results.empty());
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.fillRoundedRect(localRect(), theme.windowRadius, theme.secondBackground, panelCorners());
        painter.strokeRoundedRect(localRect(), theme.windowRadius, theme.text.withAlpha(0.12), painter.borderWidth(1),
                                  panelCorners());
    }

private:
    ScrollView* m_scroll;
    Column* m_rows;
    StyledLabel* m_empty;
    std::vector<TimeZoneMap::Info> m_results;
    int m_current = -1;
};

// One of the zones under a click, offered on the selection's card.
class NearbyChip : public View
{
public:
    NearbyChip(TimeZoneDialog& dialog, std::string zone)
        : m_dialog(dialog)
        , m_zone(std::move(zone))
    {
        m_label = addChild<StyledLabel>(TimeZoneMap::localeTimeZoneName(m_zone));
        m_label->setFontSize(kSmallFont);
        m_label->style = [this](StyledLabel& label) {
            const Theme& theme = Theme::current();
            label.setColor(isCurrent() ? theme.highlightedText : theme.text);
        };
    }

    SizeF sizeHint() const override
    {
        const SizeF label = m_label->sizeHint();
        return {label.width + metrics::m * 2, label.height + metrics::xs * 2};
    }
    bool acceptsPointer() const override { return true; }

protected:
    bool isCurrent() const { return m_zone == m_dialog.selectedZone(); }

    void layout() override
    {
        const SizeF label = m_label->sizeHint();
        m_label->setGeometry({rounding::centeredStart(geometry().width, label.width), rounding::centeredStart(geometry().height, label.height),
                              label.width, label.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const Color color = isCurrent() ? theme.highlight : theme.text.withAlpha(m_pressed ? 0.12 : 0.06);
        painter.fillRoundedRect(localRect(), geometry().height / 2, color, panelCorners());
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        const bool clicked = m_pressed && localRect().contains(event.position);
        m_pressed = false;
        requestPaint();
        if (clicked)
            m_dialog.pickNearby(m_zone);
    }

private:
    TimeZoneDialog& m_dialog;
    std::string m_zone;
    StyledLabel* m_label;
    bool m_pressed = false;
};

// The card naming the selection, next to its pin; with more than one zone
// under the click it offers them all, floating, so nothing below moves.
class Callout : public View
{
public:
    explicit Callout(TimeZoneDialog& dialog)
        : m_dialog(dialog)
    {
        m_column = addChild<ColumnLayout>();
        m_column->setSpacing(0);
        m_name = m_column->addChild<StyledLabel>();
        m_name->setWeight(600);
        m_region = m_column->addChild<StyledLabel>();
        m_region->setFontSize(kSmallFont);
        m_region->style = [](StyledLabel& label) { label.setColor(Theme::current().disabledText); };
        m_chips = m_column->addChild<Row>();
        m_chips->setSpacing(metrics::xs);
        m_chips->setLayoutHints({.margins = {0, metrics::s, 0, 0}});
    }

    void update()
    {
        const TimeZoneMap::Info& info = m_dialog.selectedInfo();
        m_name->setText(info.name);
        m_region->setText(info.region);
        m_region->setVisible(!info.region.empty() && info.region != info.name);
        while (!m_chips->children().empty())
            m_chips->takeChild(m_chips->children().front().get());
        const auto& nearby = m_dialog.nearbyZones();
        m_chips->setVisible(nearby.size() > 1);
        if (nearby.size() > 1) {
            for (const std::string& zone : nearby)
                m_chips->addChild<NearbyChip>(m_dialog, zone);
        }
        requestLayout();
    }

    SizeF sizeHint() const override
    {
        const SizeF content = m_column->sizeHint();
        return {content.width + metrics::m * 2, content.height + metrics::s * 2};
    }
    bool acceptsPointer() const override { return true; }

protected:
    void pointerEnter() override { m_dialog.setHoveredZone({}); }

    void layout() override
    {
        // Centred in the card as anchors.centerIn centres it.
        const SizeF content = m_column->sizeHint();
        m_column->setGeometry({rounding::centeredStart(geometry().width, content.width),
                               rounding::centeredStart(geometry().height, content.height), content.width, content.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.fillRoundedRect(localRect(), theme.mediumRadius, theme.secondBackground, panelCorners());
        painter.strokeRoundedRect(localRect(), theme.mediumRadius, theme.text.withAlpha(0.1), painter.borderWidth(1),
                                  panelCorners());
    }

private:
    TimeZoneDialog& m_dialog;
    ColumnLayout* m_column;
    StyledLabel* m_name;
    StyledLabel* m_region;
    Row* m_chips;
};

// What a click would pick: a ring on the zone and its name beside it.
class HoverMarker : public View
{
public:
    HoverMarker()
    {
        m_label = addChild<StyledLabel>();
        m_label->setFontSize(kSmallFont);
    }

    void setZone(const std::string& zone, PointF point, double areaWidth)
    {
        m_label->setText(zone.empty() ? std::string() : TimeZoneMap::localeTimeZoneName(zone));
        m_point = point;
        m_areaWidth = areaWidth;
        requestLayout();
        requestPaint();
    }

protected:
    void layout() override
    {
        const SizeF label = m_label->sizeHint();
        const double width = label.width + metrics::m;
        const double height = label.height + metrics::xs;
        const bool flip = m_point.x + metrics::s + width > m_areaWidth;
        m_pill = {flip ? m_point.x - metrics::s - width : m_point.x + metrics::s,
                  std::max(0.0, m_point.y - height - metrics::xs), width, height};
        m_label->setGeometry({m_pill.x + rounding::centeredStart(width, label.width), m_pill.y + rounding::centeredStart(height, label.height),
                              label.width, label.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const RectF ring{m_point.x - 6, m_point.y - 6, 12, 12};
        painter.save();
        painter.multiplyOpacity(0.8);
        painter.strokeRoundedRect(ring, 6, theme.highlight, painter.borderWidth(2));
        painter.restore();
        painter.fillRoundedRect(m_pill, m_pill.height / 2, color(SettingsColor::TimeZoneMapPill), panelCorners());
    }

private:
    StyledLabel* m_label;
    PointF m_point;
    double m_areaWidth = 0;
    RectF m_pill;
};

// The world map: land and every zone as a dot, so it is clear where a click
// lands; the ones keeping the selected zone's clock right now stand out.
class MapArea : public View
{
public:
    explicit MapArea(TimeZoneDialog& dialog)
        : m_dialog(dialog)
    {
        setClipsChildren(true);
        m_points = m_dialog.map().points();
        m_callout = addChild<Callout>(dialog);
        m_hover = addChild<HoverMarker>();
        m_hover->setVisible(false);
        setTracksHover(true);
    }

    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    void update()
    {
        m_callout->update();
        requestLayout();
        requestPaint();
    }

    void ripple()
    {
        if (!m_rippleAnimation && window()) {
            m_rippleAnimation = std::make_unique<Animation>(window());
            // A few rings for each new selection, then still: a looping ripple
            // would redraw the window every frame.
            m_rippleAnimation->setDuration(std::chrono::milliseconds(2000));
            m_rippleAnimation->setEasing(Animation::Easing::Linear);
            m_rippleAnimation->setLoops(3);
            m_rippleAnimation->onUpdate = [this](double value) {
                const double elapsed = value * 2000;
                const double progress = outCubic(std::min(1.0, elapsed / 1600));
                m_rippleScale = 1 + (3.2 - 1) * progress;
                m_rippleOpacity = 0.45 * (1 - progress);
                requestPaint();
            };
        }
        if (m_rippleAnimation)
            m_rippleAnimation->start();
        else
            m_pendingRipple = true;
    }

    void previewAt(PointF position)
    {
        if (m_callout->isVisible() && m_callout->geometry().contains(position)) {
            m_dialog.setHoveredZone({});
            return;
        }
        const RectF map = mapRect();
        m_dialog.setHoveredZone(m_dialog.map().nearest(int(position.x - map.x), int(position.y - map.y),
                                                       int(map.width), int(map.height)));
    }

    RectF mapRect() const
    {
        const double width = geometry().width;
        const double height = geometry().height;
        const double mapWidth = std::min(width, height * kMapAspect);
        const double mapHeight = mapWidth / kMapAspect;
        return {(width - mapWidth) / 2, (height - mapHeight) / 2, mapWidth, mapHeight};
    }

    // Where the zone is drawn; (-1, -1) if unknown.
    PointF pointOf(const std::string& zone) const
    {
        const RectF map = mapRect();
        if (zone.empty() || map.width <= 0)
            return {-1, -1};
        const PointF point = m_dialog.map().position(zone, int(map.width), int(map.height));
        if (point.x < 0)
            return point;
        return {map.x + point.x, map.y + point.y};
    }

protected:
    void layout() override
    {
        if (m_pendingRipple && window()) {
            m_pendingRipple = false;
            ripple();
        }
        const PointF target = pointOf(m_dialog.selectedZone());
        if (target.x != m_pinTarget.x || target.y != m_pinTarget.y) {
            const bool animate = m_dialog.animatesPin() && m_pinTarget.x >= 0 && target.x >= 0 && window();
            m_pinTarget = target;
            if (animate) {
                if (!m_pinAnimation) {
                    m_pinAnimation = std::make_unique<Animation>(window());
                    m_pinAnimation->setDuration(std::chrono::milliseconds(250));
                    m_pinAnimation->setEasing(Animation::Easing::OutCubic);
                    m_pinAnimation->onUpdate = [this](double value) {
                        m_pin = {m_pinFrom.x + (m_pinTarget.x - m_pinFrom.x) * value,
                                 m_pinFrom.y + (m_pinTarget.y - m_pinFrom.y) * value};
                        requestLayout();
                        requestPaint();
                    };
                }
                m_pinFrom = m_pin;
                m_pinAnimation->start();
            } else {
                if (m_pinAnimation)
                    m_pinAnimation->stop();
                m_pin = target;
            }
        }

        const bool pinVisible = m_pinTarget.x >= 0;
        m_callout->setVisible(pinVisible);
        if (pinVisible) {
            // Right of the pin and above it, turned back when that would leave the map.
            const SizeF size = m_callout->sizeHint();
            const bool flipX = m_pin.x + metrics::m + size.width > geometry().width;
            const bool flipY = m_pin.y - metrics::m - size.height < 0;
            m_callout->setGeometry({m_pin.x + (flipX ? -metrics::m - size.width : metrics::m),
                                    m_pin.y + (flipY ? metrics::m : -metrics::m - size.height), size.width,
                                    size.height});
        }

        // Above the pin and its card, so a zone right next to the selection can still be seen.
        const std::string& hovered = m_dialog.hoveredZone();
        const PointF hoverPoint = pointOf(hovered);
        const bool hoverVisible = !hovered.empty() && hovered != m_dialog.selectedZone() && hoverPoint.x >= 0;
        m_hover->setVisible(hoverVisible);
        m_hover->setGeometry(localRect());
        if (hoverVisible)
            m_hover->setZone(hovered, hoverPoint, geometry().width);
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const RectF map = mapRect();
        // Land in the text colour and borders in the card's, both toned down
        // so the pins and the dots stay what the eye finds first.
        painter.save();
        painter.multiplyOpacity(theme.darkMode ? 0.24 : 0.18);
        painter.drawSvg(dataPath(theme.darkMode ? "images/dark/world.svg" : "images/light/world.svg"), map);
        painter.restore();

        const int selectedOffset = m_dialog.selectedInfo().offsetSeconds;
        const bool hasSelection = !m_dialog.selectedInfo().id.empty();
        for (int pass = 0; pass < 2; ++pass) {
            for (const TimeZoneMap::Point& point : m_points) {
                const bool sameClock = hasSelection && point.offsetSeconds == selectedOffset;
                if (sameClock != (pass == 1))
                    continue;
                const PointF centre{map.x + point.x * map.width, map.y + point.y * map.height};
                painter.fillPath(circle(centre, sameClock ? 2.5 : 1.5),
                                 sameClock ? theme.highlight.withAlpha(0.55) : theme.text.withAlpha(0.18));
            }
        }

        if (m_pinTarget.x < 0)
            return;
        if (m_rippleOpacity > 0)
            painter.fillPath(circle(m_pin, 7 * m_rippleScale), theme.highlight.withAlpha(m_rippleOpacity));
        const RectF pin{m_pin.x - 7, m_pin.y - 7, 14, 14};
        painter.fillRoundedRect(pin, 7, theme.highlight);
        painter.strokeRoundedRect(pin, 7, Color::rgb(0xFFFFFF), painter.borderWidth(2));
    }

    // The zone a click would pick follows the pointer, but not over the
    // selection's card, whose own hover hides it.
    void hoverMove(const PointerEvent& event) override { previewAt(event.position); }
    void pointerMove(const PointerEvent& event) override { previewAt(event.position); }

    void hoverChanged() override
    {
        if (!isHovered())
            m_dialog.setHoveredZone({});
    }

    void pointerPress(const PointerEvent& event) override { m_pressed = event.button == MouseButton::Left; }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed || !localRect().contains(event.position))
            return;
        m_pressed = false;
        const RectF map = mapRect();
        m_dialog.mapClicked(int(event.position.x - map.x), int(event.position.y - map.y), int(map.width),
                            int(map.height));
    }

private:
    TimeZoneDialog& m_dialog;
    std::vector<TimeZoneMap::Point> m_points;
    HoverMarker* m_hover;
    Callout* m_callout;
    std::unique_ptr<Animation> m_pinAnimation;
    std::unique_ptr<Animation> m_rippleAnimation;
    PointF m_pin{-1, -1};
    PointF m_pinFrom;
    PointF m_pinTarget{-1, -1};
    double m_rippleScale = 1;
    double m_rippleOpacity = 0;
    bool m_pendingRipple = false;
    bool m_pressed = false;
};

// The map's card.
class MapCard : public View
{
public:
    explicit MapCard(TimeZoneDialog& dialog)
    {
        area = addChild<MapArea>(dialog);
    }

    MapArea* area;

protected:
    void layout() override
    {
        area->setGeometry(localRect().adjusted(metrics::m, metrics::m, -metrics::m, -metrics::m));
    }
};

} // namespace

// The title bar's band: the title at the left and the search box in the
// middle of the window, not in the middle of what the title leaves free.
class Band : public View
{
public:
    Band()
    {
        title = addChild<Label>(tr("Time Zone"));
        title->setFontSize(14);
        title->setWeight(600);
        search = addChild<SearchBox>();
    }

    Label* title;
    SearchBox* search;

    SizeF sizeHint() const override { return {0, kHeaderHeight}; }

protected:
    void layout() override
    {
        const double height = geometry().height;
        const SizeF size = title->sizeHint();
        title->setGeometry({0, rounding::centeredStart(height, size.height), size.width, size.height});
        search->setGeometry({rounding::centeredStart(geometry().width, kSearchWidth),
                             rounding::centeredStart(height, kSearchHeight), kSearchWidth, kSearchHeight});
    }
};

class TimeZoneDialog::Root : public View
{
public:
    explicit Root(TimeZoneDialog& dialog)
        : m_dialog(dialog)
    {
        // Under the band's title and search box, which take their own presses.
        m_header = addChild<WindowHeader>(kHeaderHeight);
        m_header->setMinimizeVisible(false);
        m_header->setMaximizeVisible(false);
        m_header->setButtonMargin(metrics::dialogWindowButtonsMargin);

        m_column = addChild<ColumnLayout>();
        m_column->setSpacing(metrics::dialogSpacing);
        m_column->setPadding({0, 0, 0, metrics::dialogPadding});
        m_band = m_column->addChild<Band>();
        // Shares the title bar's band, centred on the close button.
        m_band->setLayoutHints({.fillWidth = true,
                                .margins = {metrics::dialogPadding, 0, metrics::dialogPadding,
                                            metrics::s - metrics::dialogSpacing}});
        search = m_band->search;
        m_map = m_column->addChild<MapCard>(dialog);
        m_map->setLayoutHints({.fillWidth = true, .fillHeight = true});

        // The selection spelled out: its clock, where it is, and how it
        // differs from the zone in use. Always two lines, so the bar never
        // changes height.
        RowLayout* bar = m_column->addChild<RowLayout>();
        bar->setSpacing(metrics::l);
        bar->setLayoutHints({.fillWidth = true, .margins = {metrics::dialogPadding, 0, metrics::dialogPadding, 0}});
        m_time = bar->addChild<StyledLabel>();
        m_time->setFontSize(26);
        m_time->setWeight(300);
        m_time->setFontFeatures({{"tnum", 1}});
        ColumnLayout* lines = bar->addChild<ColumnLayout>();
        lines->setSpacing(metrics::xxs);
        lines->setLayoutHints({.fillWidth = true});
        m_place = lines->addChild<StyledLabel>();
        m_place->setLayoutHints({.fillWidth = true});
        m_detail = lines->addChild<StyledLabel>();
        m_detail->setFontSize(kSmallFont);
        m_detail->setLayoutHints({.fillWidth = true});
        m_detail->style = [](StyledLabel& label) { label.setColor(Theme::current().disabledText); };
        RowLayout* buttons = bar->addChild<RowLayout>();
        buttons->setSpacing(metrics::dialogButtonSpacing);
        m_cancel = buttons->addChild<DialogButton>(tr("Cancel"));
        m_cancel->onClicked = [this] { m_dialog.close(); };
        // TRANSLATORS: the button that makes the chosen time zone the system's.
        m_set = buttons->addChild<DialogButton>(tr("Set"), true);
        m_set->onClicked = [this] { m_dialog.apply(); };

        results = addChild<ResultsPopup>();
        results->setVisible(false);
    }

    SearchBox* search;
    ResultsPopup* results;

    MapArea* map() const { return m_map->area; }

    void update()
    {
        const TimeZoneMap::Info& info = m_dialog.selectedInfo();
        m_time->setText(info.time);
        const bool named = !info.name.empty();
        m_place->setText(!named ? tr("Click the map or search to choose a time zone")
                         : !info.region.empty() && info.region != info.name
                                 // TRANSLATORS: a time zone's city and its country or region, such as "Paris, France".
                                 ? format(tr("{city}, {region}"), {{"city", info.name}, {"region", info.region}})
                                 : info.name);
        m_place->setWeight(named ? 600 : 400);
        m_place->style = [named](StyledLabel& label) {
            const Theme& theme = Theme::current();
            label.setColor(named ? theme.text : theme.disabledText);
        };
        if (!named)
            m_detail->setText({});
        else if (!m_dialog.isChanged())
            m_detail->setText(tr("Current time zone"));
        else
            m_detail->setText(joined({info.day, info.difference,
                                      format(tr("Currently {zone}"), {{"zone", m_dialog.map().currentTimeZone()}})},
                                     "  ·  "));
        m_set->setEnabled(m_dialog.isChanged());
        map()->update();
        requestLayout();
    }

    void showResults(bool visible)
    {
        results->setVisible(visible);
        requestLayout();
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        m_header->setGeometry({0, 0, geometry().width, kHeaderHeight});
        m_column->setGeometry(localRect());
    }

    void childrenLaidOut() override
    {
        // Wider than the field: each result carries a time and a day.
        const RectF field = search->geometry();
        const PointF band = m_band->geometry().topLeft();
        results->setGeometry({band.x + field.x + (kSearchWidth - kResultsWidth) / 2,
                              band.y + field.bottom() + metrics::xs, kResultsWidth, results->popupHeight()});
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        showResults(false);
        // The dialog moves from anywhere on its background.
        if (Window* window = this->window())
            window->startMove();
    }

private:
    TimeZoneDialog& m_dialog;
    WindowHeader* m_header;
    ColumnLayout* m_column;
    Band* m_band;
    MapCard* m_map;
    StyledLabel* m_time;
    StyledLabel* m_place;
    StyledLabel* m_detail;
    DialogButton* m_cancel;
    DialogButton* m_set;
};

TimeZoneDialog::TimeZoneDialog(SettingsWindow& parent, TimeZoneMap& map)
    : m_map(map)
    , m_window(std::make_unique<Window>())
{
    auto root = std::make_unique<Root>(*this);
    m_root = root.get();
    m_window->setTitle(tr("Time Zone"));
    m_window->setBackgroundColor(withAlpha(ColorId::SecondBackground, 0.5));
    m_window->setContentView(std::move(root));
    m_window->setTransientParent(&parent.window(), false);
    m_window->onCloseRequested = [this] { close(); };
    m_window->resize({kWidth, kHeight});
    m_window->setMinimumSize({kWidth, kHeight});
    m_window->setMaximumSize({kWidth, kHeight});

    TextField* field = m_root->search->field;
    field->onTextChanged = [this, field] {
        const std::string text = field->text();
        m_root->results->setResults(m_map.search(text));
        const bool any = text.find_first_not_of(" \t\n\r") != std::string::npos;
        m_root->showResults(any);
    };
    field->onAccepted = [this] { pickResult(); };
    m_root->search->onKey = [this](uint32_t key) {
        ResultsPopup* results = m_root->results;
        const int count = results->count();
        if (key == key::Down) {
            if (count > 0)
                results->setCurrentIndex((results->currentIndex() + 1) % count);
            return true;
        }
        if (key == key::Up) {
            if (count > 0)
                results->setCurrentIndex((results->currentIndex() - 1 + count) % count);
            return true;
        }
        if (key == key::Escape && results->isVisible()) {
            m_root->showResults(false);
            return true;
        }
        return false;
    };
    m_root->results->onPicked = [this](int index) {
        m_root->results->setCurrentIndex(index);
        pickResult();
    };

    m_subscriptions.push_back(m_map.availableListChanged.connect([this] {
        m_nearbyZones = m_map.availableList();
        m_selectedZone = m_nearbyZones.empty() ? std::string() : m_nearbyZones.front();
        updateInfo();
        selectionStarted.emit();
    }));
    m_subscriptions.push_back(m_map.currentTimeZoneChanged.connect([this] { updateInfo(); }));
    m_subscriptions.push_back(selectionStarted.connect([this] { m_root->map()->ripple(); }));
    m_tick.setInterval(std::chrono::seconds(15));
    m_tick.onTimeout = [this] { updateInfo(); };
}

TimeZoneDialog::~TimeZoneDialog() = default;

void TimeZoneDialog::show()
{
    if (!m_window)
        return;
    m_window->setBackgroundBlur({0, 0, kWidth, kHeight}, Theme::current().windowRadius);
    m_animatePin = false;
    m_hoveredZone.clear();
    m_map.select(m_map.currentTimeZoneId());
    m_window->show();
    m_root->search->field->setFocus();
    m_tick.start();
}

void TimeZoneDialog::close()
{
    if (!m_window)
        return;
    m_tick.stop();
    // The window may be handling the event that closes it.
    std::shared_ptr<Window> window(std::move(m_window));
    base::EventLoop::current()->post([window] {});
    if (onClosed)
        onClosed();
}

bool TimeZoneDialog::isChanged() const
{
    return !m_selectedZone.empty() && m_selectedZone != m_map.currentTimeZoneId();
}

void TimeZoneDialog::updateInfo()
{
    m_selectedInfo = m_selectedZone.empty() ? TimeZoneMap::Info() : m_map.info(m_selectedZone);
    if (m_root)
        m_root->update();
    changed.emit();
}

void TimeZoneDialog::pickResult()
{
    ResultsPopup* results = m_root->results;
    const int index = results->currentIndex();
    if (index < 0 || index >= results->count())
        return;
    const std::string zone = results->results()[size_t(index)].id;
    m_animatePin = true;
    m_root->showResults(false);
    m_root->search->field->setText({});
    m_map.select(zone);
}

void TimeZoneDialog::pickNearby(const std::string& id)
{
    m_animatePin = true;
    if (m_selectedZone == id)
        return;
    m_selectedZone = id;
    updateInfo();
    selectionStarted.emit();
}

void TimeZoneDialog::mapClicked(int x, int y, int width, int height)
{
    m_animatePin = true;
    m_root->showResults(false);
    m_map.clicked(x, y, width, height);
}

void TimeZoneDialog::setHoveredZone(std::string id)
{
    if (id == m_hoveredZone)
        return;
    m_hoveredZone = std::move(id);
    m_root->map()->update();
}

void TimeZoneDialog::apply()
{
    m_map.setTimeZone(m_selectedZone);
    close();
}

} // namespace settings
