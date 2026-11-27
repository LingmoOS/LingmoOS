// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/launcher/launcher_page_indicator.h"

#include <algorithm>
#include <cmath>

#include "ui/painter.h"

namespace Lingmo::Shell {

static const ui::Color kWhite {255, 255, 255, 255};

static int jsRound(double value)
{
    return int(std::floor(value + 0.5));
}

LauncherPageIndicator::LauncherPageIndicator()
{
    setName("launcher-folder-indicator");
    setOpacity(0);
}

void LauncherPageIndicator::setDotSize(double size)
{
    if (size == m_dotSize)
        return;
    m_dotSize = size;
    requestPaint();
}

void LauncherPageIndicator::setDots(int count, int current, bool animate)
{
    const double opacity = count > 1 ? 1 : 0;
    if (count != m_count) {
        m_count = count;
        m_alphas.assign(size_t(std::max(0, count)), 0.5);
        if (current >= 0 && current < count)
            m_alphas[size_t(current)] = 1;
        m_current = current;
        requestPaint();
    }
    if (opacity != m_opacityTween.target()
            || (!m_opacityTween.isRunning() && this->opacity() != opacity))
        m_opacityTween.start(animate ? window() : nullptr, this->opacity(), opacity,
                             std::chrono::milliseconds(200), easing::outCubic,
                             [this](double value) { setOpacity(value); });
    if (current == m_current)
        return;
    m_current = current;
    const std::vector<double> from = m_alphas;
    m_colorTween.start(animate ? window() : nullptr, 0, 1, std::chrono::milliseconds(160),
                       easing::linear, [this, from](double t) {
                           for (size_t i = 0; i < m_alphas.size() && i < from.size(); ++i) {
                               const double target = int(i) == m_current ? 1 : 0.5;
                               m_alphas[i] = from[i] + (target - from[i]) * t;
                           }
                           requestPaint();
                       });
}

double LauncherPageIndicator::spacing() const
{
    return jsRound(m_dotSize * 1.4);
}

ui::SizeF LauncherPageIndicator::implicitSize() const
{
    const int n = std::max(0, m_count);
    return {n * m_dotSize + std::max(0, n - 1) * spacing() + 2 * kPadding,
            m_dotSize + 2 * kPadding};
}

ui::RectF LauncherPageIndicator::dotRect(int index) const
{
    return {kPadding + index * (m_dotSize + spacing()), kPadding, m_dotSize, m_dotSize};
}

void LauncherPageIndicator::paint(ui::Painter &painter)
{
    for (int i = 0; i < m_count; ++i) {
        const ui::RectF dot = dotRect(i);
        painter.drawSmoothRectangle(dot, dot.width / 2, kWhite.withAlpha(m_alphas[size_t(i)]));
    }
}

void LauncherPageIndicator::pick(ui::PointF position)
{
    for (int i = 0; i < m_count; ++i) {
        const ui::RectF dot = dotRect(i);
        const double half = spacing() / 2;
        if (position.x >= dot.x - half && position.x < dot.right() + half) {
            if (onPick)
                onPick(i);
            return;
        }
    }
}

} // namespace Lingmo::Shell
