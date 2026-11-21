// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/progress_bar.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkSamplingOptions.h"

#include "ui/animation.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace Lingmo::ui {

ProgressBar::ProgressBar() = default;

ProgressBar::~ProgressBar() = default;

void ProgressBar::setValue(double value)
{
    value = std::clamp(value, 0.0, 1.0);
    if (value == m_value)
        return;
    m_value = value;
    requestPaint();
}

void ProgressBar::setIndeterminate(bool indeterminate)
{
    if (indeterminate == m_indeterminate)
        return;
    m_indeterminate = indeterminate;
    if (indeterminate && window()) {
        if (!m_animation) {
            m_animation = std::make_unique<Animation>(window());
            m_animation->setDuration(std::chrono::milliseconds(1000));
            m_animation->setEasing(Animation::Easing::InOutCubic);
            m_animation->setLoops(Animation::Infinite);
            m_animation->setAlternating(true);
            m_animation->onUpdate = [this](double value) {
                m_phase = value;
                requestPaint();
            };
        }
        m_animation->start();
    } else if (m_animation) {
        m_animation->stop();
    }
    requestPaint();
}

void ProgressBar::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const RectF bounds = localRect();
    const double radius = std::min(bounds.width, bounds.height) / 2;
    painter.fillRoundedRect(bounds, radius, theme.alternateBackground);
    if (m_indeterminate) {
        const double width = std::max(bounds.height, bounds.width / 3);
        painter.fillRoundedRect({(bounds.width - width) * m_phase, 0, width, bounds.height}, radius,
                                theme.highlight);
    } else if (m_value > 0) {
        // Never shorter than a dot, so a little progress still shows.
        const double width = std::max(bounds.height, bounds.width * m_value);
        painter.fillRoundedRect({0, 0, width, bounds.height}, radius, theme.highlight);
    }
}

BusyIndicator::BusyIndicator() = default;

BusyIndicator::~BusyIndicator() = default;

void BusyIndicator::setRunning(bool running)
{
    m_running = running;
    if (m_animation) {
        if (running)
            m_animation->start();
        else
            m_animation->stop();
    }
    requestPaint();
}

void BusyIndicator::setDuration(std::chrono::milliseconds duration)
{
    m_duration = duration;
    if (m_animation)
        m_animation->setDuration(duration);
}

void BusyIndicator::setColor(std::optional<Color> color)
{
    m_color = color;
    requestPaint();
}

void BusyIndicator::paint(Painter& painter)
{
    if (!m_running)
        return;
    if (!m_animation && window()) {
        m_animation = std::make_unique<Animation>(window());
        m_animation->setDuration(m_duration);
        m_animation->setEasing(Animation::Easing::Linear);
        m_animation->setLoops(Animation::Infinite);
        m_animation->onUpdate = [this](double value) {
            // Hidden, it stops ticking, as the RotationAnimator stops with
            // its item or window; the next paint starts it again.
            for (const View* view = this; view; view = view->parent()) {
                if (!view->isVisible()) {
                    m_animation->stop();
                    return;
                }
            }
            m_angle = value * 360;
            requestPaint();
        };
        m_animation->start();
    } else if (m_animation && !m_animation->isRunning()) {
        m_animation->start();
    }

    const Rect box = painter.deviceRect(localRect());
    const int side = std::min(box.width, box.height);
    sk_sp<SkImage> glyph = IconLoader::instance().glyph("refresh", side, m_color.value_or(Theme::current().text));
    if (!glyph)
        return;
    // Turned, so sampled smoothly rather than snapped like still icons.
    SkCanvas* canvas = painter.canvas();
    canvas->save();
    canvas->translate(SkScalar(box.x + box.width / 2.0), SkScalar(box.y + box.height / 2.0));
    canvas->rotate(SkScalar(m_angle));
    SkPaint paint;
    paint.setAlphaf(float(painter.opacity()));
    canvas->drawImage(glyph, -side / 2.0f, -side / 2.0f,
                      SkSamplingOptions(SkFilterMode::kLinear), &paint);
    canvas->restore();
}

} // namespace Lingmo::ui
