// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"
#include "base/signal.h"

namespace settings {

// The dock's settings: read from the shell's file, changed through the dock,
// which owns that file.
class DockSettings
{
public:
    DockSettings();

    bool efficientMode() const { return m_efficientMode; }
    bool modeAvailable() const { return m_modeAvailable; }
    bool modePending() const { return m_modePending; }
    const std::string &modeError() const { return m_modeError; }
    void setEfficientMode(bool enabled);
    Lingmo::base::Signal<> modeChanged;
    int iconSize() const { return m_iconSize; }
    void setIconSize(int size);

    // 0 left, 1 bottom, 2 right.
    int direction() const { return m_direction; }
    void setDirection(int direction);

    // 0 always shown, 1 always hidden, 2 smart hide.
    int visibility() const { return m_visibility; }
    void setVisibility(int visibility);

    double glassTint() const { return m_glassTint; }
    void setGlassTint(double tint);

    bool minimizedArea() const { return m_minimizedArea; }
    void setMinimizedArea(bool enabled);

    // The icons grow under the pointer.
    bool magnification() const { return m_magnification; }
    void setMagnification(bool enabled);
    // The largest scale they grow to; 1 to 2.
    double magnificationFactor() const { return m_magnificationFactor; }
    void setMagnificationFactor(double factor);

    // An application that is starting hops a few times.
    bool bounceOnLaunch() const { return m_bounceOnLaunch; }
    void setBounceOnLaunch(bool enabled);
    int taskbarAlignment() const { return m_taskbarAlignment; }
    void setTaskbarAlignment(int value);
    int taskbarSearch() const { return m_taskbarSearch; }
    void setTaskbarSearch(int value);

    // 0 = Normal (scale), 1 = Genie Effect.
    int minimizeStyle() const { return m_minimizeStyle; }
    void setMinimizeStyle(int style);

private:
    void readMode();
    bool m_efficientMode = false;
    bool m_modeAvailable = false;
    bool m_modePending = false;
    std::string m_modeError;
    uint64_t m_modeGeneration = 0;
    Lingmo::base::dbus::NameWatch m_modeOwner;
    Lingmo::base::dbus::Match m_modeProperties;
    Lingmo::base::dbus::PendingCall m_modeRead;
    Lingmo::base::dbus::PendingCall m_modeWrite;
    Lingmo::base::dbus::Proxy m_dock;
    int m_iconSize = 0;
    int m_direction = 0;
    int m_visibility = 0;
    double m_glassTint = 0.5;
    bool m_minimizedArea = true;
    bool m_magnification = true;
    double m_magnificationFactor = 1.5;
    bool m_bounceOnLaunch = true;
    int m_taskbarAlignment = 0;
    int m_taskbarSearch = 0;
    int m_minimizeStyle = 0;
};

} // namespace settings
