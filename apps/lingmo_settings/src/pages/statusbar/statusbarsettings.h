// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"

namespace settings {

// The top bar's settings: the Gaussian blur behind it and how transparent that
// blur is. Read from the shell's file, changed through org.lingmo.Statusbar,
// which owns it.
class StatusBarSettings
{
public:
    // The compositor's defaults, so a page without a shell.conf shows the same.
    static constexpr double kDefaultBlurSigma = 20.0;
    static constexpr double kDefaultOpacity = 0.55;
    static constexpr double kMaxBlurSigma = 64.0;

    StatusBarSettings();

    double blurSigma() const { return m_blurSigma; }
    // 0 for a clear bar; logical pixels of Gaussian sigma.
    void setBlurSigma(double sigma);

    double opacity() const { return m_opacity; }
    // 0..1; the page shows it as transparency, 1 - opacity.
    void setOpacity(double opacity);

private:
    Lingmo::base::dbus::Proxy m_statusBar;
    double m_blurSigma = kDefaultBlurSigma;
    double m_opacity = kDefaultOpacity;
};

} // namespace settings
