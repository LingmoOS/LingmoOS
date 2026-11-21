// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/osd/level_track.h"

#include <algorithm>
#include <cmath>

#include "ui/animation.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "toolkit/compositor_colors.h"

namespace Lingmo::Shell {

LevelTrack::LevelTrack() = default;

LevelTrack::~LevelTrack() = default;

void LevelTrack::setPosition(double position)
{
    position = std::clamp(position, 0.0, 1.0);
    if (position == m_target)
        return;
    m_target = position;
    if (!m_animated || !window()) {
        if (m_animation)
            m_animation->stop();
        m_drawn = position;
        requestPaint();
        return;
    }
    if (!m_animation) {
        m_animation = std::make_unique<ui::Animation>(window());
        m_animation->setDuration(std::chrono::milliseconds(120));
        m_animation->setEasing(ui::Animation::Easing::OutCubic);
        m_animation->onUpdate = [this](double value) {
            m_drawn = m_from + (m_target - m_from) * value;
            requestPaint();
        };
    }
    // A retarget restarts from what is drawn.
    m_animation->stop();
    m_from = m_drawn;
    m_animation->start();
}

void LevelTrack::setIcon(std::string file)
{
    if (file == m_icon)
        return;
    m_icon = std::move(file);
    requestPaint();
}

void LevelTrack::paint(ui::Painter &painter)
{
    const bool dark = ui::Theme::current().darkMode;
    const ui::RectF rect = localRect();
    const double height = rect.height;
    const double travel = rect.width - height;
    const double hairline = 1 / painter.scale();
    const ui::Color outline = ui::color(toolkit::CompositorColor::LevelTrackOutline);
    const ui::RoundedShape pill {height / 2, 0, ui::Corners::Circular};

    painter.drawRoundedRect(rect, pill, ui::color(toolkit::CompositorColor::LevelTrackBackground), hairline,
                            outline);

    const ui::Color fill = ui::color(toolkit::CompositorColor::LevelTrackFill);
    const ui::RectF fillRect {0, 0, height + travel * m_drawn, height};
    painter.drawRoundedRect(fillRect, pill, fill, hairline, outline);

    // The knob, lifted by a shadow: FishUI's DropShadow of radius 3 works in
    // device pixels, a deviation of (3 dpr + 1) / 3.3333 of them.
    const ui::RectF knob {fillRect.width - height, 0, height, height};
    const double sigma = (3 * painter.scale() + 1) / 3.3333 / painter.scale();
    painter.drawShadow(knob, height / 2, sigma, ui::color(toolkit::CompositorColor::LevelTrackKnobShadow),
                       {0, 1});
    painter.drawRoundedRect(knob, pill,
                            ui::color(toolkit::CompositorColor::LevelTrackKnob), hairline,
                            outline);

    if (!m_icon.empty()) {
        painter.save();
        painter.multiplyOpacity(dark ? 0.85 : 0.7);
        const ui::RectF icon {(height - kIconSize) / 2, (height - kIconSize) / 2, kIconSize,
                              kIconSize};
        painter.drawSvg(m_icon, icon,
                        dark ? std::optional<ui::Color>(ui::Color::rgb(0xFFFFFF)) : std::nullopt);
        painter.restore();
    }
}

} // namespace Lingmo::Shell
