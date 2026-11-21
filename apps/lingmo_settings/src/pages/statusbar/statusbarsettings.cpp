// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/statusbar/statusbarsettings.h"

#include <algorithm>

#include "base/paths.h"
#include "base/settings.h"

using namespace Lingmo::base;

namespace settings {

StatusBarSettings::StatusBarSettings()
    : m_statusBar(dbus::Connection::session(), "org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar")
{
    const Settings shell(paths::lingmoConfig("shell"));
    const double blur = shell.doubleValue("Statusbar/BlurSigma", kDefaultBlurSigma);
    m_blurSigma = std::clamp(blur, 0.0, kMaxBlurSigma);
    const double opacity = shell.doubleValue("Statusbar/Opacity", kDefaultOpacity);
    m_opacity = std::clamp(opacity, 0.0, 1.0);
}

// Called on every slider move; the bar does not wait for a reply.
void StatusBarSettings::setBlurSigma(double sigma)
{
    sigma = std::clamp(sigma, 0.0, kMaxBlurSigma);
    if (sigma == m_blurSigma)
        return;
    m_statusBar.send("setBlurSigma", sigma);
    m_blurSigma = sigma;
}

void StatusBarSettings::setOpacity(double opacity)
{
    opacity = std::clamp(opacity, 0.0, 1.0);
    if (opacity == m_opacity)
        return;
    m_statusBar.send("setOpacity", opacity);
    m_opacity = opacity;
}

} // namespace settings
