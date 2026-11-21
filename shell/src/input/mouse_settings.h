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

class MouseSettings
{
public:
    explicit MouseSettings(const std::string &settingsFile = {});
    ~MouseSettings();
    bool registerService();

    bool available() const { return m_available; }

    bool shakeToLocate() const { return m_shakeToLocate; }
    void setShakeToLocate(bool enabled);
    base::Signal<> shakeToLocateChanged;

    bool leftHanded() const { return m_leftHanded; }

    bool naturalScroll() const { return m_naturalScroll; }

    bool acceleration() const { return m_acceleration; }

    double pointerAcceleration() const { return m_pointerAcceleration; }

    void setAvailable(bool available);

    void setLeftHanded(bool enabled);
    void setNaturalScroll(bool enabled);
    void setAcceleration(bool enabled);
    void setPointerAcceleration(double value);

    base::Signal<> availableChanged;
    base::Signal<> leftHandedChanged;
    base::Signal<> naturalScrollChanged;
    base::Signal<> accelerationChanged;
    base::Signal<> pointerAccelerationChanged;
    base::Signal<> settingsChanged;

private:
    template<typename T>
    void save(const std::string &key, T value);
    void notify(const base::Signal<> &signal, const char *member);

    base::Settings m_settings;
    bool m_available = false;
    bool m_shakeToLocate = true;
    bool m_leftHanded = false;
    // A wheel reads as a scrollbar, not as a finger on the content: pushing it
    // down moves the content down, as on every other desktop. A touchpad keeps
    // the natural direction, where the content follows the fingers, so the two
    // default apart.
    bool m_naturalScroll = false;
    bool m_acceleration = true;
    double m_pointerAcceleration = 0;
    std::unique_ptr<DBusExport> m_object;
};

} // namespace Lingmo::Input
