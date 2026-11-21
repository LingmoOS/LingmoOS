// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/status_bar_backdrop.h"

#include "shell/wallpaper/wallpaper.h"

namespace Lingmo::Shell {

StatusBarBackdrop::StatusBarBackdrop() = default;

StatusBarBackdrop::~StatusBarBackdrop() = default;

void StatusBarBackdrop::follow(Wallpaper *wallpaper)
{
    m_judgementChanged.reset();
    m_wallpaper = wallpaper;
    if (!m_wallpaper) { setUnavailable(); return; }
    const auto update = [this] {
        m_valid=m_wallpaper->isJudged();m_style=m_wallpaper->style();changed();
    };
    m_judgementChanged = m_wallpaper->judgementChanged.connect(update);
    update();
}

void StatusBarBackdrop::present(bool valid, const Style &style)
{
    if (valid == m_valid && style == m_style)
        return;
    m_valid = valid;
    m_style = style;
    changed();
}

void StatusBarBackdrop::setColor(const ui::Color &color)
{
    present(true, BarBackdrop::styleForSample(BarBackdrop::sampleColor(color)));
}

void StatusBarBackdrop::setUnavailable()
{
    present(false, Style());
}

} // namespace Lingmo::Shell
