// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/timer.h"
#include "ui/animation.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/view.h"

#include <chrono>
#include <cmath>
#include <functional>
#include <memory>
#include <string>

namespace Lingmo::ui {
class ScrollView;
}

namespace views
{

using Lingmo::ui::Color;
using Lingmo::ui::RectF;
using Lingmo::ui::SizeF;

// A font size in points, in the toolkit's logical pixels.
inline double points(double pointSize)
{
    return pointSize * 4 / 3;
}

inline constexpr Color white(double alpha = 1)
{
    return Color{255, 255, 255, uint8_t(alpha * 255 + 0.5)};
}

inline constexpr Color rgba(double r, double g, double b, double a)
{
    return Color{uint8_t(r * 255 + 0.5), uint8_t(g * 255 + 0.5), uint8_t(b * 255 + 0.5), uint8_t(a * 255 + 0.5)};
}

// Where a layout centres an item of the extent: its size rounded up, its
// position rounded (a 17-high child in 44 lands at 14).
inline double layoutCenter(double parentExtent, double itemExtent)
{
    return Lingmo::ui::rounding::halfAwayFromZero((parentExtent - std::ceil(itemExtent)) / 2);
}

// The application's glyphs, registered once as "vp-<name>".
void registerGlyphs();
std::string glyph(const std::string& name);

// A label: size in points (0 for the theme's), weight 0 for normal.
Lingmo::ui::Label* addLabel(Lingmo::ui::View* parent, std::string text, double pointSize = 0,
                              std::optional<Lingmo::ui::ColorRecipe> color = std::nullopt, int weight = 0);
// Tabular figures: the OpenType "tnum" feature.
void setTabular(Lingmo::ui::Label* label);

// The plates over the video draw no body of their own when the compositor
// frosts the video behind them (a Wayland window); headless and in tests
// they are solid plates.
bool compositorGlass();
void paintPlate(Lingmo::ui::Painter& painter, const RectF& rect, double radius);

// A view taking a click: pressed, hovered and released inside call onClicked.
class Clickable : public Lingmo::ui::View
{
public:
    bool acceptsPointer() const override { return isEnabled(); }
    bool isPressed() const { return m_pressed; }
    bool isHoveredNow() const { return m_hovered; }

    std::function<void()> onClicked;
    // Pressing with the right button.
    std::function<void(Lingmo::ui::PointF windowPosition)> onRightClicked;
    std::function<void()> onDoubleClicked;
    std::function<void()> onHoverChanged;

protected:
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const Lingmo::ui::PointerEvent& event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent& event) override;
    void enabledChanged() override;

    bool m_pressed = false;
    bool m_hovered = false;
};

// A white glyph on the dark plates, with a round hover
// plate, and a tool tip after 800 ms.
class ControlButton : public Clickable
{
public:
    explicit ControlButton(std::string icon, double size = 36, double iconSize = 20);

    void setIcon(std::string icon);
    void setChecked(bool checked);
    void setToolTipText(std::string text);
    SizeF sizeHint() const override { return {m_size, m_size}; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const Lingmo::ui::PointerEvent& event) override;
    void enabledChanged() override;

private:
    void hideToolTip();

    Lingmo::ui::Icon* m_glyph;
    std::string m_icon;
    double m_size;
    double m_iconSize;
    bool m_checked = false;
    std::string m_toolTip;
    bool m_toolTipShown = false;
    Lingmo::base::Timer m_toolTipTimer;
};

// A text button on the dark plates.
class PillButton : public Clickable
{
public:
    explicit PillButton(std::string text, bool prominent = false);

    void setText(std::string text);
    void setProminent(bool prominent);
    SizeF sizeHint() const override;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    Lingmo::ui::Label* m_label;
    bool m_prominent;
};

// A number animated towards a target on the window's frame clock: a new
// target starts from where the value is.
class Tween
{
public:
    Tween(double value, std::chrono::milliseconds duration,
          Lingmo::ui::Animation::Easing easing = Lingmo::ui::Animation::Easing::Linear);
    ~Tween();

    double value() const { return m_value; }
    double target() const { return m_to; }
    bool isRunning() const;
    // Without a window (headless before show) the value jumps.
    void setTarget(Lingmo::ui::View* owner, double target);
    void jumpTo(double value);

    std::function<void()> onUpdate;

private:
    std::unique_ptr<Lingmo::ui::Animation> m_animation;
    Lingmo::ui::Window* m_window = nullptr;
    std::chrono::milliseconds m_duration;
    Lingmo::ui::Animation::Easing m_easing;
    double m_value;
    double m_from;
    double m_to;
};

// A separator line one device pixel thick.
void paintSeparator(Lingmo::ui::Painter& painter, double x, double y, double width, Color color);

// The view's scroll bars drawn for the dark surfaces under them.
void darkenBars(Lingmo::ui::ScrollView* view);

} // namespace views
