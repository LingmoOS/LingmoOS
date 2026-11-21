// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_controls.h"

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "shell/notification/notification_motion.h"
#include "ui/application.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

static double linear(double channel)
{
    const double c = channel / 255.0;
    return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

bool inkIsDark(ui::Color surface)
{
    const double luminance = 0.2126 * linear(surface.r) + 0.7152 * linear(surface.g) + 0.0722 * linear(surface.b);
    // White's contrast against it reaches 3:1.
    return 1.05 / (luminance + 0.05) >= 3;
}

ui::Color inkOn(ui::Color surface, double alpha)
{
    const ui::Color ink = inkIsDark(surface) ? ui::Color {255, 255, 255, 255} : ui::Color {0, 0, 0, 255};
    return ink.withAlpha(alpha);
}

ui::Color rgba(double red, double green, double blue, double alpha)
{
    const auto channel = [](double value) { return uint8_t(std::lround(std::clamp(value, 0.0, 1.0) * 255)); };
    return {channel(red), channel(green), channel(blue), channel(alpha)};
}

ui::Color pillFill(ui::Color surface, bool hovered, bool pressed)
{
    if (inkIsDark(surface))
        return inkOn(surface, pressed ? 0.3 : hovered ? 0.24 : 0.16);
    return inkOn(surface, pressed ? 0.16 : hovered ? 0.12 : 0.07);
}

// The application font's 0.9, as a whole logical pixel size.
static double smallFontSize()
{
    return ui::Theme::current().fontPointSize * 0.9 * 4 / 3;
}

// A 1 px bar `length` long through `centre`, turned by 45 degrees either
// way: a rotated rectangle of height 1, antialiased.
static void paintCross(ui::Painter &painter, ui::PointF centre, double length, ui::Color color)
{
    const double half = length / 2;
    const double thickness = 0.5;
    for (const double sign : {1.0, -1.0}) {
        // Along the bar and across it.
        const ui::PointF along {half / std::sqrt(2.0), sign * half / std::sqrt(2.0)};
        const ui::PointF across {-sign * thickness / std::sqrt(2.0), thickness / std::sqrt(2.0)};
        ui::Path path;
        path.addPolygon({centre - along - across, centre + along - across, centre + along + across,
                         centre - along + across});
        painter.fillPath(path, color);
    }
}

ClearButton::ClearButton(ui::Window *window, Kind kind)
    : m_kind(kind)
    , m_confirm(kind == Kind::Clear)
    , m_progress(std::make_unique<Tween>(window, std::chrono::milliseconds(240), outCubic))
{
    m_label = addChild<ui::Label>(tr("Clear"));
    m_label->setWeight(500);
    m_label->setElide(ui::Label::Elide::None);
    m_label->setVisible(false);
    setClipsChildren(true);
    m_progress->onChanged = [this] {
        updateLabel();
        requestPaint();
        if (onWidthChanged)
            onWidthChanged();
    };
    m_languageChanged = ui::Application::instance()->languageChanged.connect([this] {
        m_label->setText(tr("Clear"));
        requestLayout();
        if (onWidthChanged)
            onWidthChanged();
    });
}

ClearButton::~ClearButton() = default;

void ClearButton::setConfirm(bool confirm)
{
    m_confirm = confirm;
    if (!confirm)
        setArmed(false);
}

double ClearButton::progress() const
{
    return m_progress->value();
}

void ClearButton::setArmed(bool armed)
{
    if (armed == m_armed)
        return;
    m_armed = armed;
    // A NumberAnimation restarted from where the value is.
    m_progress->setTarget(armed ? 1 : 0, true);
    if (onArmedChanged)
        onArmedChanged();
}

void ClearButton::disarmUnless(ui::PointF windowPoint)
{
    if (!m_armed)
        return;
    if (!isVisible() || !localRect().contains(mapFromWindow(windowPoint)))
        setArmed(false);
}

void ClearButton::reset()
{
    m_armed = false;
    m_hovered = false;
    m_pressed = false;
    m_progress->snap(0);
    updateLabel();
}

double ClearButton::buttonHeight() const
{
    return m_kind == Kind::Clear ? 26 : 24;
}

double ClearButton::currentWidth() const
{
    const double height = buttonHeight();
    m_label->setFontSize(smallFontSize());
    const double label = m_label->sizeHint().width;
    return height + (label + ui::Theme::current().largeSpacing - height) * progress();
}

void ClearButton::updateLabel()
{
    const double opacity = std::max(0.0, (progress() - 0.3) / 0.7);
    m_label->setOpacity(opacity);
    m_label->setVisible(opacity > 0);
}

void ClearButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void ClearButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void ClearButton::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void ClearButton::pointerRelease(const ui::PointerEvent &event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (!localRect().contains(event.position))
        return;
    if (m_confirm && !m_armed) {
        setArmed(true);
        return;
    }
    setArmed(false);
    if (onClicked)
        onClicked();
}

void ClearButton::enabledChanged()
{
    // A disabled clear button backs out.
    if (!isEnabled() && m_kind == Kind::Clear)
        setArmed(false);
}

void ClearButton::layout()
{
    m_label->setFontSize(smallFontSize());
    const ui::SizeF size = m_label->sizeHint();
    const double margin = ui::Theme::current().largeSpacing / 2;
    m_label->setGeometry({geometry().width - margin - size.width, (geometry().height - size.height) / 2, size.width,
                          size.height});
}

void ClearButton::paint(ui::Painter &painter)
{
    const ui::Theme &theme = ui::Theme::current();
    const ui::RectF rect = localRect();
    const double radius = rect.height / 2;
    if (m_kind == Kind::Clear) {
        painter.drawSmoothRectangle(rect, radius, pillFill(theme.background, m_hovered, m_pressed));
    } else if ((m_hovered || progress() > 0) && m_tint) {
        // Not snapped: the glyph is not either.
        painter.drawSmoothRectangle(rect, radius, m_tint(m_pressed ? 2 : 1));
    }

    const double glyphOpacity = 1 - std::min(1.0, progress() / 0.5);
    if (glyphOpacity <= 0)
        return;
    painter.save();
    painter.multiplyOpacity(glyphOpacity);
    const double side = rect.height;
    if (m_kind == Kind::Clear) {
        // Centred on whole logical pixels.
        const double length = 7 * std::sqrt(2.0);
        const double x = std::round((side - length) / 2);
        const double y = std::round((side - 1) / 2);
        const bool dark = inkIsDark(theme.background);
        paintCross(painter, {x + length / 2, y + 0.5}, length, inkOn(theme.background, dark ? 0.85 : 0.65));
    } else {
        painter.multiplyOpacity(0.7);
        paintCross(painter, {side / 2, side / 2}, 8 * std::sqrt(2.0), theme.text);
    }
    painter.restore();
}

PillButton::PillButton(std::string text)
{
    m_label = addChild<ui::Label>(std::move(text));
    m_label->setWeight(500);
    m_label->setElide(ui::Label::Elide::None);
}

void PillButton::setText(std::string text)
{
    m_label->setText(std::move(text));
    requestLayout();
}

double PillButton::preferredWidth() const
{
    m_label->setFontSize(smallFontSize());
    return m_label->sizeHint().width + ui::Theme::current().largeSpacing * 2;
}

void PillButton::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void PillButton::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void PillButton::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left)
        return;
    m_pressed = true;
    requestPaint();
}

void PillButton::pointerRelease(const ui::PointerEvent &event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    requestPaint();
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

void PillButton::layout()
{
    m_label->setFontSize(smallFontSize());
    const ui::SizeF size = m_label->sizeHint();
    m_label->setGeometry({(geometry().width - size.width) / 2, (geometry().height - size.height) / 2, size.width,
                          size.height});
}

void PillButton::paint(ui::Painter &painter)
{
    const ui::RectF rect = localRect();
    painter.drawSmoothRectangle(rect, rect.height / 2, pillFill(ui::Theme::current().background, m_hovered, m_pressed));
}

} // namespace Lingmo::Shell
