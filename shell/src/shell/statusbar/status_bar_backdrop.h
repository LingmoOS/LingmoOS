// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include "base/signal.h"
#include "shell/wallpaper/bar_backdrop.h"
#include "ui/geometry.h"

namespace Lingmo::Shell {

class Wallpaper;

// What lies behind the status bar: the wallpaper's top strip under the bar
// decides whether the bar's contents are light or dark. The compositor's
// wallpaper judges what it shows; the bar follows that verdict, in the frame
// the picture changes.
class StatusBarBackdrop
{
public:
    using Sample = BarBackdrop::Sample;
    using Style = BarBackdrop::Style;

    StatusBarBackdrop();
    ~StatusBarBackdrop();

    StatusBarBackdrop(const StatusBarBackdrop &) = delete;
    StatusBarBackdrop &operator=(const StatusBarBackdrop &) = delete;

    // Whether a wallpaper or colour has been judged; otherwise the bar
    // follows the system's dark mode.
    bool isValid() const { return m_valid; }
    bool useLightForeground() const { return m_style.useLightForeground; }

    // Follows what the desktop shows; null stops.
    void follow(Wallpaper *wallpaper);
    // A verdict given directly.
    void present(bool valid, const Style &style);
    void setColor(const ui::Color &color);
    void setUnavailable();

    base::Signal<> changed;

    static Sample sampleImage(const ui::Bitmap &screenImage, int barHeight)
    {
        return BarBackdrop::sampleImage(screenImage, barHeight);
    }
    static Style styleForSample(const Sample &sample) { return BarBackdrop::styleForSample(sample); }

private:
    Wallpaper *m_wallpaper = nullptr;
    base::Subscription m_judgementChanged;
    bool m_valid = false;
    Style m_style;
};

} // namespace Lingmo::Shell

