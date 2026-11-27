// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"

namespace Lingmo::Shell {

// The dock's [Dock] group of shell.conf: icon size, edge, visibility, glass
// tint and the minimized area. Missing keys are written with their
// defaults. Read on the file thread;
// the defaults stand until isReady().
class DockSettings
{
public:
    enum Direction {
        Left = 0,
        Bottom,
        Right,
    };

    enum Visibility {
        AlwaysShow = 0,
        AlwaysHide,
        IntellHide,
    };

    // The icon size the dock's metrics scale from.
    static constexpr int kBaseIconSize = 54;
    // The icon size of the efficient taskbar, where the bar is a full-width
    // strip rather than a floating dock.
    static constexpr int kTaskbarIconSize = 44;

    // `path` is shell.conf.
    explicit DockSettings(std::string path);

    bool isReady() const { return m_ready; }

    bool efficientMode() const { return m_efficientMode; }
    void setEfficientMode(bool enabled);
    base::Signal<> efficientModeChanged;
    static constexpr int taskbarHeight = 64;
    // The taskbar's corner radius, mirrored on its bottom corners as the
    // strip stands against the screen edge.
    static constexpr int taskbarRadius = 28;
    enum TaskbarAlignment { AlignLeft = 0, AlignCenter = 1 };
    enum TaskbarSearch { SearchHidden = 0, SearchShown = 1 };
    TaskbarAlignment taskbarAlignment() const { return m_taskbarAlignment; }
    void setTaskbarAlignment(TaskbarAlignment alignment);
    TaskbarSearch taskbarSearch() const { return m_taskbarSearch; }
    void setTaskbarSearch(TaskbarSearch search);
    int iconSize() const { return m_efficientMode ? kTaskbarIconSize : m_iconSize; }
    // Written once the size has stopped changing for a moment, as a slider
    // drags it.
    void setIconSize(int iconSize);
    Direction direction() const { return m_efficientMode ? Bottom : m_direction; }
    void setDirection(Direction direction);
    Visibility visibility() const { return m_efficientMode ? AlwaysShow : m_visibility; }
    void setVisibility(Visibility visibility);
    int edgeMargins() const { return m_efficientMode ? 0 : m_edgeMargins; }
    double glassTint() const { return m_glassTint; }
    // Clamped to 0..1; not a number is the default.
    void setGlassTint(double tint);
    bool minimizedArea() const { return !m_efficientMode && m_minimizedArea; }
    void setMinimizedArea(bool enabled);

    // The icons grow under the pointer, as macOS magnification.
    bool magnification() const { return !m_efficientMode && m_magnification; }
    void setMagnification(bool enabled);
    // The largest scale the pointer brings an icon to; 1 to 2.
    double magnificationFactor() const { return m_magnificationFactor; }
    void setMagnificationFactor(double factor);
    // An application that is starting hops a few times.
    bool bounceOnLaunch() const { return !m_efficientMode && m_bounceOnLaunch; }
    void setBounceOnLaunch(bool enabled);

    base::Signal<> ready;
    base::Signal<> iconSizeChanged;
    base::Signal<> directionChanged;
    base::Signal<> visibilityChanged;
    base::Signal<> glassTintChanged;
    base::Signal<> minimizedAreaChanged;
    base::Signal<> magnificationChanged;
    base::Signal<> magnificationFactorChanged;
    base::Signal<> bounceOnLaunchChanged;
    base::Signal<> taskbarAlignmentChanged;
    base::Signal<> taskbarSearchChanged;

private:
    struct Values {
        bool efficientMode = false;
        int iconSize = kBaseIconSize;
        Direction direction = Bottom;
        Visibility visibility = AlwaysShow;
        int edgeMargins = 10;
        double glassTint = 0.5;
        bool minimizedArea = true;
        bool magnification = true;
        double magnificationFactor = 1.5;
        bool bounceOnLaunch = true;
        TaskbarAlignment taskbarAlignment = AlignLeft;
        TaskbarSearch taskbarSearch = SearchHidden;
    };

    static Values load(const std::string &path);
    void write(const char *key, double value);

    std::string m_path;
    bool m_ready = false;
    bool m_efficientMode = false;
    bool m_modeSet = false;
    int m_iconSize = kBaseIconSize;
    Direction m_direction = Bottom;
    Visibility m_visibility = AlwaysShow;
    int m_edgeMargins = 10;
    double m_glassTint = 0.5;
    bool m_minimizedArea = true;
    bool m_magnification = true;
    double m_magnificationFactor = 1.5;
    bool m_bounceOnLaunch = true;
    TaskbarAlignment m_taskbarAlignment = AlignLeft;
    TaskbarSearch m_taskbarSearch = SearchHidden;
    base::Timer m_iconSizeSave;
    base::Job m_load;
};

} // namespace Lingmo::Shell

