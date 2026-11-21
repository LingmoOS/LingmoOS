// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

#include "base/settings.h"
#include "base/signal.h"

namespace Lingmo::Input {

class DBusExport;

// The touchpad half of Chromium's TouchpadSettingsEvdev, plus libinput's
// disable-while-typing and disable-with-an-external-mouse modes. Exported as
// org.lingmo.Touchpad1 for Settings.
class TouchpadSettings
{
public:
    // libinput reports two-finger scrolling unaccelerated at about 10.5 units
    // per millimetre of finger travel, and clients move content one logical
    // pixel per unit. The factor scales that; the default half puts content
    // under the fingers on a 150% laptop panel.
    static constexpr double kDefaultScrollSpeed = 0.5;
    static constexpr double kMinimumScrollSpeed = 0.1;
    static constexpr double kMaximumScrollSpeed = 2.0;

    explicit TouchpadSettings(const std::string &settingsFile = {});
    ~TouchpadSettings();
    bool registerService();

    bool available() const { return m_available; }

    bool enabled() const { return m_enabled; }

    bool disableWithMouse() const { return m_disableWithMouse; }

    bool disableWhileTyping() const { return m_disableWhileTyping; }

    bool tapToClick() const { return m_tapToClick; }

    bool tapDragging() const { return m_tapDragging; }

    bool naturalScroll() const { return m_naturalScroll; }

    double pointerAcceleration() const { return m_pointerAcceleration; }

    double scrollSpeed() const { return m_scrollSpeed; }

    // A touchpad offers both two-finger press and bottom-right corner press.
    bool secondaryClickConfigurable() const { return m_secondaryClickConfigurable; }

    bool twoFingerSecondaryClick() const { return m_twoFingerSecondaryClick; }

    void setAvailable(bool available);
    void setSecondaryClickConfigurable(bool configurable);

    void setEnabled(bool enabled);
    void setDisableWithMouse(bool enabled);
    void setDisableWhileTyping(bool enabled);
    void setTapToClick(bool enabled);
    void setTapDragging(bool enabled);
    void setNaturalScroll(bool enabled);
    void setPointerAcceleration(double value);
    void setScrollSpeed(double value);
    void setTwoFingerSecondaryClick(bool enabled);

    base::Signal<> availableChanged;
    base::Signal<> enabledChanged;
    base::Signal<> disableWithMouseChanged;
    base::Signal<> disableWhileTypingChanged;
    base::Signal<> tapToClickChanged;
    base::Signal<> tapDraggingChanged;
    base::Signal<> naturalScrollChanged;
    base::Signal<> pointerAccelerationChanged;
    base::Signal<> scrollSpeedChanged;
    base::Signal<> secondaryClickConfigurableChanged;
    base::Signal<> twoFingerSecondaryClickChanged;
    base::Signal<> settingsChanged;

private:
    template<typename T>
    void save(const std::string &key, T value);
    void notify(const base::Signal<> &signal, const char *member);

    base::Settings m_settings;
    bool m_available = false;
    bool m_enabled = true;
    bool m_disableWithMouse = false;
    bool m_disableWhileTyping = true;
    bool m_tapToClick = false;
    bool m_tapDragging = false;
    bool m_naturalScroll = true;
    double m_pointerAcceleration = 0;
    double m_scrollSpeed = kDefaultScrollSpeed;
    bool m_secondaryClickConfigurable = false;
    bool m_twoFingerSecondaryClick = true;
    std::unique_ptr<DBusExport> m_object;
};

} // namespace Lingmo::Input
