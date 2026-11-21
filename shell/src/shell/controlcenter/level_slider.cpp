// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/controlcenter/level_slider.h"

#include <algorithm>
#include <cmath>

#include "shell/controlcenter/panel_style.h"
#include "shell/osd/level_track.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace Lingmo::Shell {

// How long the knob keeps the user's value for the service to come back.
static constexpr std::chrono::milliseconds kSettle {300};

double snapToDevice(double value, double scale)
{
    return std::round(value * scale) / scale;
}

double alignedLabelTop(const ui::Label *label, double textBaseline, double scale)
{
    return snapToDevice(textBaseline - label->baselineOffset(), scale);
}

double alignedLabelHeight(const ui::Label *label, double textBaseline, double scale)
{
    return label->sizeHint().height + alignedLabelTop(label, textBaseline, scale);
}

static double windowScale(const ui::View *view)
{
    const ui::Window *window = view->window();
    return window && window->scale() > 0 ? window->scale() : 1;
}

// Wheel notches in 120ths, positive away from the user; a touchpad's pixels
// count twelve times as much.
static double angleNotches(const ui::ScrollEvent &event)
{
    const double y = event.value120Y != 0 ? -event.value120Y : -event.deltaY * 12;
    const double x = event.value120X != 0 ? -event.value120X : -event.deltaX * 12;
    return (y != 0 ? y : -x) / 120;
}

LevelSlider::LevelSlider()
{
    m_track = addChild<LevelTrack>();
    m_settle.setSingleShot(true);
    m_settle.setInterval(kSettle);
    m_settle.onTimeout = [this] {
        m_settling = false;
        m_gliding = false;
        // The binding takes the service's value again.
        if (!m_pressed)
            setVisualValue(m_value);
    };
}

void LevelSlider::setRange(double from, double to, double stepSize)
{
    m_from = from;
    m_to = to;
    m_stepSize = stepSize;
    m_track->setPosition(position());
}

void LevelSlider::setValue(double value)
{
    m_value = value;
    if (!m_pressed && !m_settling)
        setVisualValue(value);
}

double LevelSlider::position() const
{
    return m_to > m_from ? std::clamp((m_visualValue - m_from) / (m_to - m_from), 0.0, 1.0) : 0;
}

void LevelSlider::setIcon(std::string file)
{
    m_track->setIcon(std::move(file));
}

void LevelSlider::setVisualValue(double value)
{
    m_visualValue = value;
    // Only wheel steps glide; a drag and the service's values snap.
    m_track->setAnimated(m_gliding && !m_pressed);
    m_track->setPosition(position());
}

void LevelSlider::setFromFraction(double fraction)
{
    fraction = std::clamp(fraction, 0.0, 1.0);
    double value = m_from + fraction * (m_to - m_from);
    if (m_stepSize > 0)
        value = m_from + std::round((value - m_from) / m_stepSize) * m_stepSize;
    value = std::clamp(value, std::min(m_from, m_to), std::max(m_from, m_to));
    if (value == m_visualValue)
        return;
    m_settling = true;
    setVisualValue(value);
    m_settle.start();
    if (onMoved)
        onMoved(value);
}

void LevelSlider::wheel(const ui::ScrollEvent &event)
{
    if (m_to == m_from)
        return;
    const double step = m_stepSize > 0 ? m_stepSize : (m_to - m_from) / 20;
    m_gliding = true;
    setFromFraction(position() + angleNotches(event) * step * 5 / (m_to - m_from));
}

void LevelSlider::dragTo(double x)
{
    const double travel = geometry().width - geometry().height;
    if (travel > 0)
        setFromFraction((x - geometry().height / 2) / travel);
}

void LevelSlider::layout()
{
    m_track->setGeometry(localRect());
}

void LevelSlider::pointerPress(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left)
        return;
    m_pressed = true;
    dragTo(event.position.x);
}

void LevelSlider::pointerMove(const ui::PointerEvent &event)
{
    if (m_pressed)
        dragTo(event.position.x);
}

void LevelSlider::pointerRelease(const ui::PointerEvent &event)
{
    if (event.button != ui::MouseButton::Left || !m_pressed)
        return;
    m_pressed = false;
    if (!m_settling)
        setVisualValue(m_value);
}

bool LevelSlider::scroll(const ui::ScrollEvent &event)
{
    wheel(event);
    return true;
}

TileSlider::TileSlider(std::string title)
{
    m_title = addChild<ui::Label>(std::move(title));
    m_title->setElide(ui::Label::Elide::Right);
    m_slider = addChild<LevelSlider>();
}

void TileSlider::setTitle(std::string title)
{
    m_title->setText(std::move(title));
    requestLayout();
}

// The capitals' top, not the line box, keeps the padding from the tile's edge.
double TileSlider::heightForWidth(double) const
{
    const double padding = ui::Theme::current().largeSpacing;
    const double spacing = ui::Theme::current().smallSpacing;
    return padding + alignedLabelHeight(m_title, m_title->capHeight(), windowScale(this)) + spacing
            + LevelSlider::kHeight + padding;
}

ui::SizeF TileSlider::sizeHint() const
{
    return {0, heightForWidth(0)};
}

void TileSlider::layout()
{
    const double padding = ui::Theme::current().largeSpacing;
    const double spacing = ui::Theme::current().smallSpacing;
    const double scale = windowScale(this);
    const double width = geometry().width - 2 * padding;
    const double labelTop = alignedLabelTop(m_title, m_title->capHeight(), scale);
    const double labelHeight = m_title->sizeHint().height;
    m_title->setGeometry({padding + 3, padding + labelTop, width - 3, labelHeight});
    m_slider->setGeometry({padding, padding + labelTop + labelHeight + spacing, width, LevelSlider::kHeight});
}

void TileSlider::paint(ui::Painter &painter)
{
    ::Lingmo::Shell::Panel::paintTile(painter, localRect());
}

bool TileSlider::scroll(const ui::ScrollEvent &event)
{
    m_slider->wheel(event);
    return true;
}

} // namespace Lingmo::Shell
