// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets.h"
#include "icons.h"

#include "ui/application.h"
#include "ui/icon.h"
#include "ui/cell_layout.h"
#include "ui/scroll_bar.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <cmath>

using namespace Lingmo::ui;
using namespace std::chrono_literals;

namespace views
{

void registerGlyphs()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;
    for (const auto& [name, svg] : embeddedIcons())
        Icon::registerGlyph(glyph(std::string(name)), std::string(svg));
}

std::string glyph(const std::string& name)
{
    return "vp-" + name;
}

Label* addLabel(View* parent, std::string text, double pointSize, std::optional<ColorRecipe> color, int weight)
{
    auto* label = parent->addChild<Label>(std::move(text));
    if (pointSize > 0)
        label->setFontSize(points(pointSize));
    if (color)
        label->setColor(color);
    if (weight)
        label->setWeight(weight);
    label->setElide(Label::Elide::None);
    return label;
}

void setTabular(Label* label)
{
    label->setFontFeatures({{"tnum", 1}});
}

bool compositorGlass()
{
    const Application* application = Application::instance();
    return application && application->platform() == Application::Platform::Wayland;
}

void paintPlate(Painter& painter, const RectF& rect, double radius)
{
    const Corners corners = Theme::current().continuousCorners ? Corners::Continuous : Corners::Circular;
    const Color hairline = white(0.12);
    if (compositorGlass()) {
        // The compositor's glass is the body; only the hairline is ours.
        painter.drawRoundedRect(rect, {radius, 0, corners}, Color{0, 0, 0, 0}, 1 / painter.scale(), hairline);
        return;
    }
    painter.drawRoundedRect(rect, {radius, 0, corners}, rgba(0.1, 0.1, 0.11, 0.85), 1 / painter.scale(), hairline);
}

void paintSeparator(Painter& painter, double x, double y, double width, Color color)
{
    painter.drawRoundedRect({x, y, width, 1 / painter.scale()}, {0, 0, Corners::Circular}, color);
}

void darkenBars(ScrollView* view)
{
    for (const auto& child : view->children()) {
        if (auto* bar = dynamic_cast<ScrollBar*>(child.get()))
            bar->setDarkMode(true);
    }
}

void Clickable::pointerEnter()
{
    m_hovered = true;
    requestPaint();
    if (onHoverChanged)
        onHoverChanged();
}

void Clickable::pointerLeave()
{
    m_hovered = false;
    requestPaint();
    if (onHoverChanged)
        onHoverChanged();
}

void Clickable::pointerPress(const PointerEvent& event)
{
    if (event.button == MouseButton::Right) {
        if (onRightClicked)
            onRightClicked(mapToWindow(event.position));
        return;
    }
    if (event.button != MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
    if (event.clickCount == 2 && onDoubleClicked)
        onDoubleClicked();
}

void Clickable::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

void Clickable::enabledChanged()
{
    m_pressed = false;
    requestPaint();
}

ControlButton::ControlButton(std::string icon, double size, double iconSize)
    : m_glyph(addChild<Icon>(icon.empty() ? std::string() : glyph(icon), iconSize))
    , m_icon(std::move(icon))
    , m_size(size)
    , m_iconSize(iconSize)
{
    m_glyph->setColor(white());
    m_toolTipTimer.setSingleShot(true);
    m_toolTipTimer.onTimeout = [this] {
        if (!m_hovered || m_pressed || m_toolTip.empty() || !window())
            return;
        window()->pinToolTip(this, localRect(), m_toolTip);
        m_toolTipShown = true;
    };
}

void ControlButton::setIcon(std::string icon)
{
    if (m_icon == icon)
        return;
    m_icon = std::move(icon);
    m_glyph->setIconName(m_icon.empty() ? std::string() : glyph(m_icon));
}

void ControlButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;
    m_checked = checked;
    requestPaint();
}

void ControlButton::setToolTipText(std::string text)
{
    m_toolTip = std::move(text);
}

void ControlButton::hideToolTip()
{
    m_toolTipTimer.stop();
    if (m_toolTipShown && window())
        window()->unpinToolTip(this);
    m_toolTipShown = false;
}

void ControlButton::pointerEnter()
{
    Clickable::pointerEnter();
    if (!m_toolTip.empty())
        m_toolTipTimer.start(800ms);
}

void ControlButton::pointerLeave()
{
    Clickable::pointerLeave();
    hideToolTip();
}

void ControlButton::pointerPress(const PointerEvent& event)
{
    hideToolTip();
    Clickable::pointerPress(event);
}

void ControlButton::enabledChanged()
{
    Clickable::enabledChanged();
    setOpacity(isEnabled() ? 1 : 0.4);
    hideToolTip();
}

void ControlButton::layout()
{
    const RectF rect = localRect();
    m_glyph->setGeometry({rounding::centeredStart(rect.width, m_iconSize), rounding::centeredStart(rect.height, m_iconSize),
                          m_iconSize, m_iconSize});
}

void ControlButton::paint(Painter& painter)
{
    const RectF rect = localRect();
    const double alpha = m_pressed ? 0.22 : m_hovered || m_checked ? 0.12 : 0;
    if (alpha > 0)
        painter.drawRoundedRect(rect, {rect.height / 2, 0, Corners::Circular}, white(alpha));
}

PillButton::PillButton(std::string text, bool prominent)
    : m_label(addLabel(this, std::move(text), 0, white(), 500))
    , m_prominent(prominent)
{
}

void PillButton::setText(std::string text)
{
    m_label->setText(std::move(text));
    requestLayout();
}

void PillButton::setProminent(bool prominent)
{
    if (m_prominent == prominent)
        return;
    m_prominent = prominent;
    requestPaint();
}

SizeF PillButton::sizeHint() const
{
    return {m_label->sizeHint().width + 24, 28};
}

void PillButton::layout()
{
    // The content item fills the button; the text is centred in it, unsnapped.
    m_label->setTextAlignment(Label::TextAlignment::Center);
    m_label->setVerticallyCentered(true);
    m_label->setGeometry(localRect());
}

void PillButton::paint(Painter& painter)
{
    const RectF rect = localRect();
    const Color highlight = Theme::current().highlight;
    Color fill;
    if (m_prominent)
        fill = highlight.withAlpha(m_pressed ? 0.8 * highlight.a / 255.0 : highlight.a / 255.0);
    else
        fill = white(m_pressed ? 0.3 : m_hovered ? 0.24 : 0.16);
    painter.drawRoundedRect(rect, {rect.height / 2, 0, Corners::Circular}, fill);
}

Tween::Tween(double value, std::chrono::milliseconds duration, Animation::Easing easing)
    : m_duration(duration)
    , m_easing(easing)
    , m_value(value)
    , m_from(value)
    , m_to(value)
{
}

Tween::~Tween() = default;

bool Tween::isRunning() const
{
    return m_animation && m_animation->isRunning();
}

void Tween::jumpTo(double value)
{
    if (m_animation)
        m_animation->stop();
    m_from = m_to = m_value = value;
    if (onUpdate)
        onUpdate();
}

void Tween::setTarget(View* owner, double target)
{
    if (target == m_to)
        return;
    Window* window = owner ? owner->window() : nullptr;
    if (!window) {
        jumpTo(target);
        return;
    }
    if (!m_animation || m_window != window) {
        m_animation = std::make_unique<Animation>(window);
        m_window = window;
        m_animation->setDuration(m_duration);
        m_animation->setEasing(m_easing);
        m_animation->onUpdate = [this](double t) {
            m_value = m_from + (m_to - m_from) * t;
            if (onUpdate)
                onUpdate();
        };
    }
    m_animation->stop();
    m_from = m_value;
    m_to = target;
    m_animation->start();
}

} // namespace views
