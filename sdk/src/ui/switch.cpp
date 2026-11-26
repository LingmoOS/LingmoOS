// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/color_provider.h"
#include "ui/switch.h"

#include <algorithm>
#include <chrono>

#include "ui/animation.h"
#include "ui/painter.h"

namespace Lingmo::ui {

static constexpr SizeF kIndicator = {38, 20};
static constexpr double kHandle = 16;
static constexpr double kHandleMargin = 2;

static Color mix(Color from, Color to, double t)
{
    const auto channel = [t](uint8_t a, uint8_t b) { return uint8_t(a + (b - a) * t + 0.5); };
    return {channel(from.r, to.r), channel(from.g, to.g), channel(from.b, to.b), channel(from.a, to.a)};
}

Switch::Switch(std::string text)
    : ToggleControl(std::move(text), kIndicator, 8, Kind::Switch)
{
}

Switch::~Switch() = default;

void Switch::checkedChanged(bool animated)
{
    const double target = isChecked() ? 1 : 0;
    if (!animated || !window()) {
        // A state set from code, such as a page loading its values, snaps.
        m_handle = m_color = target;
        return;
    }
    if (!m_handleAnimation) {
        m_handleAnimation = std::make_unique<Animation>(window());
        m_handleAnimation->setDuration(std::chrono::milliseconds(250));
        m_handleAnimation->setEasing(Animation::Easing::OutSine);
        m_handleAnimation->onUpdate = [this](double value) {
            m_handle = m_handleFrom + ((isChecked() ? 1 : 0) - m_handleFrom) * value;
            requestPaint();
        };
        m_colorAnimation = std::make_unique<Animation>(window());
        m_colorAnimation->setDuration(std::chrono::milliseconds(125));
        m_colorAnimation->setEasing(Animation::Easing::InOutCubic);
        m_colorAnimation->onUpdate = [this](double value) {
            m_color = m_colorFrom + ((isChecked() ? 1 : 0) - m_colorFrom) * value;
            requestPaint();
        };
    }
    m_handleFrom = m_handle;
    m_colorFrom = m_color;
    m_handleAnimation->start();
    m_colorAnimation->start();
}

void Switch::paintIndicator(Painter& painter, const RectF& rect)
{
    const Theme& theme = Theme::current();
    painter.save();
    if (!isEnabled())
        painter.multiplyOpacity(0.5);

    const Color trackColor = mix(color(ColorId::SwitchTrack), color(ColorId::SwitchTrackOn), m_color);
    painter.drawSmoothRectangle(rect, rect.height / 2, trackColor);

    if (theme.controlStyle == 1) {
        // Liquid Glass: rim highlight on top edge of track.
        const double rimA = isChecked() ? 0.18 : 0.26;
        painter.drawSmoothRectangle(
            {rect.x + 0.5, rect.y + 0.5, rect.width - 1, rect.height * 0.45},
            rect.height / 2,
            Color{255, 255, 255, uint8_t(rimA * 255)});
    }

    const double travel = rect.width - kHandle - 2 * kHandleMargin;
    const RectF handle = {rect.x + kHandleMargin + travel * m_handle,
                          rect.y + (rect.height - kHandle) / 2, kHandle, kHandle};

    if (theme.controlStyle == 1) {
        // Handle: a glassy bead with a soft drop shadow.
        painter.drawShadow(handle, kHandle / 2, 3, Color{0, 0, 0, 50}, {0, 1});
    }
    painter.drawSmoothRectangle(handle, kHandle / 2, color(ColorId::SwitchHandle));

    if (theme.controlStyle == 1) {
        // Handle top-edge specular.
        const RectF topHalf = {handle.x + 1, handle.y + 1, handle.width - 2, handle.height * 0.45};
        painter.drawSmoothRectangle(topHalf, kHandle / 2, Color{255, 255, 255, 60});
    }
    painter.restore();
}

} // namespace Lingmo::ui
