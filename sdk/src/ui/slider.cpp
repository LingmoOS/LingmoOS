// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/slider.h"

#include <algorithm>
#include <cmath>

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"

#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/smooth_rectangle.h"
#include "ui/window.h"

namespace Lingmo::ui {

static constexpr double kTrack = 4;
static constexpr double kHandle = 22;
// The handle's drop shadow: radius 10, 21 samples, four pixels down.
static constexpr double kShadowSigma = 5;
static constexpr double kShadowRadius = 10;
static constexpr double kShadowOffset = 4;

Slider::Slider() = default;

void Slider::setRange(double from, double to)
{
    m_from = from;
    m_to = to;
    setValue(m_value);
    // The handle moves for the same value in another range.
    requestPaint();
}

void Slider::setStepSize(double step)
{
    m_step = std::max(0.0, step);
    requestPaint();
}

void Slider::setStepped(bool stepped)
{
    if (stepped == m_stepped)
        return;
    m_stepped = stepped;
    requestLayout();
}

void Slider::setSnapMode(SnapMode mode)
{
    m_snapMode = mode;
}

Slider::SnapMode Slider::snapMode() const
{
    return m_snapMode.value_or(m_stepped ? SnapMode::SnapAlways : SnapMode::SnapOnRelease);
}

// A value set from outside is clamped, not snapped.
void Slider::setValue(double value)
{
    value = std::clamp(value, std::min(m_from, m_to), std::max(m_from, m_to));
    if (value == m_value)
        return;
    m_value = value;
    requestPaint();
    updateValueToolTip();
}

void Slider::setValueToolTip(std::function<std::string(double value)> text)
{
    m_valueToolTip = std::move(text);
    updateValueToolTip();
}

void Slider::updateValueToolTip()
{
    Window* window = this->window();
    if (!window || !m_valueToolTip)
        return;
    if (m_pressed)
        window->pinToolTip(this, handleRect(), m_valueToolTip(m_value));
    else
        window->unpinToolTip(this);
}

// On a step whenever there are steps.
double Slider::valueAt(double position) const
{
    const double value = (m_to - m_from) * position;
    if (m_step <= 0)
        return m_from + value;
    return m_from + std::round(value / m_step) * m_step;
}

double Slider::snapPosition(double position) const
{
    const double range = m_to - m_from;
    if (range == 0 || m_step <= 0)
        return position;
    const double step = m_step / range;
    return std::clamp(std::round(position / step) * step, 0.0, 1.0);
}

double Slider::position() const
{
    return m_to == m_from ? 0 : std::clamp((m_value - m_from) / (m_to - m_from), 0.0, 1.0);
}

double Slider::visualPosition() const
{
    return m_pressed ? m_dragPosition : position();
}

RectF Slider::visualRect() const
{
    // The handle's shadow, wherever the handle is.
    const double reach = Painter::shadowReach(kShadowSigma);
    return localRect().adjusted(-reach, std::min(0.0, kShadowOffset - reach), reach, kShadowOffset + reach);
}

SizeF Slider::sizeHint() const
{
    return {200, m_stepped ? 32.0 : 27.0};
}

RectF Slider::handleRect() const
{
    const RectF bounds = localRect();
    const double travel = std::max(0.0, bounds.width - kHandle);
    return {travel * visualPosition(), (bounds.height - kHandle) / 2, kHandle, kHandle};
}

void Slider::paint(Painter& painter)
{
    const RectF bounds = localRect();
    const RectF handle = handleRect();
    const double centreY = bounds.height / 2;
    // The track runs between the handle's centres at both ends, its round
    // caps reaching a little past them.
    const double start = kHandle / 2 - kTrack / 2;
    const double length = std::max(0.0, bounds.width - kHandle) + kTrack;

    painter.save();
    if (!isEnabled())
        painter.multiplyOpacity(0.4);
    // Rounded rectangles, snapped as the rounded-rectangle shader draws them.
    painter.drawRoundedRect({start, centreY - kTrack / 2, length, kTrack}, {kTrack / 2}, color(ColorId::SliderTrack));
    if (visualPosition() > 0) {
        const double filled = std::max(0.0, bounds.width - kHandle) * visualPosition() + kTrack;
        painter.drawRoundedRect({start, centreY - kTrack / 2, filled, kTrack}, {kTrack / 2},
                                color(ColorId::SliderTrackFilled));
    }
    if (m_stepped && m_step > 0 && m_to != m_from) {
        const int intervals = int(std::ceil(std::abs(m_to - m_from) / m_step));
        const double travel = std::max(0.0, bounds.width - kHandle);
        // Marks only while they stay apart.
        if (intervals > 0 && intervals <= int(travel / 6)) {
            const Color mark = color(ColorId::SliderTickMark);
            for (int i = 0; i <= intervals; ++i) {
                const double x = kHandle / 2 + travel * std::min(1.0, double(i) / intervals);
                painter.drawRoundedRect({x - 0.5, centreY + 12, 1, 3}, {0.5}, mark);
            }
        }
    }
    painter.restore();

    painter.save();
    if (!isEnabled())
        painter.multiplyOpacity(0.55);
    const double scale = painter.scale();
    if (isEnabled()) {
        // DropShadow radius 10 in device pixels, deviation (radius + 1) / 3.3333.
        const double sigma = (kShadowRadius * scale + 1) / 3.3333 / scale;
        painter.drawShadow(handle, kHandle / 2, sigma,
                           color(m_pressed ? ColorId::SliderHandleShadowPressed : ColorId::SliderHandleShadow),
                           {0, kShadowOffset});
    }
    // A plain antialiased rectangle, its border not pixel aligned.
    const Color fill = color(m_pressed ? ColorId::SliderHandlePressed : ColorId::SliderHandle);
    const Color border = color(ColorId::SliderHandleBorder);
    if (!isEnabled()) {
        painter.drawSmoothRectangle(handle, kHandle / 2, fill, 0.5, border);
        painter.restore();
        return;
    }
    // Enabled, it is a layer under a drop shadow: the layer texture is
    // copied into the effect's source, padded across by the blur radius
    // (device pixels counted as logical ones), and that drawn where the
    // handle lies. Each copy filters the edge again.
    SkCanvas* canvas = painter.canvas();
    const auto surface = [canvas](int width, int height) {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(width, height);
        sk_sp<SkSurface> made = canvas->makeSurface(info);
        if (!made)
            made = SkSurfaces::Raster(info);
        if (made)
            made->getCanvas()->clear(SK_ColorTRANSPARENT);
        return made;
    };
    const double padding = kShadowRadius * scale;
    const int texture = int(std::lround(std::ceil(kHandle) * scale));
    const int padded = int(std::lround(std::ceil(kHandle + 2 * padding) * scale));
    const sk_sp<SkSurface> layer = surface(texture, texture);
    const sk_sp<SkSurface> source = surface(padded, texture);
    if (layer && source) {
        drawSmoothRoundedRectangle(layer->getCanvas(), {0, 0, kHandle, kHandle}, kHandle / 2, scale, fill, 0.5,
                                  border, 1);
        source->getCanvas()->drawImageRect(layer->makeImageSnapshot(),
                                           SkRect::MakeXYWH(SkScalar(padding * scale), 0, SkScalar(kHandle * scale),
                                                            SkScalar(kHandle * scale)),
                                           SkSamplingOptions(SkFilterMode::kLinear), nullptr);
        const PointF origin = painter.offset() + handle.topLeft();
        SkPaint paint;
        paint.setAlphaf(float(painter.opacity()));
        canvas->drawImageRect(source->makeImageSnapshot(),
                              SkRect::MakeXYWH(SkScalar((origin.x - padding) * scale), SkScalar(origin.y * scale),
                                               SkScalar((kHandle + 2 * padding) * scale), SkScalar(kHandle * scale)),
                              SkSamplingOptions(SkFilterMode::kLinear), &paint);
    }
    painter.restore();
}

void Slider::moveTo(double value)
{
    value = std::clamp(value, std::min(m_from, m_to), std::max(m_from, m_to));
    if (value == m_value)
        return;
    m_value = value;
    requestPaint();
    if (onMoved)
        onMoved(m_value);
}

void Slider::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left)
        return;
    const RectF handle = handleRect();
    m_pressed = true;
    if (onPressedChanged)
        onPressedChanged();
    m_dragPosition = position();
    // Grabbing the handle keeps it where it was taken; elsewhere it jumps there.
    m_grabOffset = handle.contains(event.position) ? event.position.x - (handle.x + kHandle / 2) : 0;
    pointerMove(event);
}

void Slider::pointerMove(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    const double travel = geometry().width - kHandle;
    if (travel <= 0)
        return;
    double fraction = std::clamp((event.position.x - m_grabOffset - kHandle / 2) / travel, 0.0, 1.0);
    if (snapMode() == SnapMode::SnapAlways)
        fraction = snapPosition(fraction);
    m_dragPosition = fraction;
    requestPaint();
    moveTo(valueAt(fraction));
    updateValueToolTip();
}

void Slider::pointerRelease(const PointerEvent&)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    if (onPressedChanged)
        onPressedChanged();
    // The handle lands where the value is: on its step unless nothing snaps.
    if (snapMode() == SnapMode::NoSnap)
        moveTo(valueAt(m_dragPosition));
    requestPaint();
    updateValueToolTip();
}

bool Slider::keyPress(const KeyEvent& event)
{
    // A step, or 0.1 without one.
    const double step = m_step > 0 ? m_step : 0.1;
    const double direction = m_to >= m_from ? 1 : -1;
    switch (event.key) {
    case key::Left:
    case key::Down:
        moveTo(m_value - step * direction);
        return true;
    case key::Right:
    case key::Up:
        moveTo(m_value + step * direction);
        return true;
    case key::Home:
        moveTo(m_from);
        return true;
    case key::End:
        moveTo(m_to);
        return true;
    default:
        return false;
    }
}

} // namespace Lingmo::ui
