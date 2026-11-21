// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "shell/common/pending_calls.h"

namespace Lingmo::Shell {

// The display backlight as lingmo-services keeps it
// (org.lingmo.Services.Brightness): its percent, whether there is one, and
// the sliders' and keys' changes. Everything is asynchronous; the setter does
// not report back, brightnessChanged does.
class Brightness
{
public:
    explicit Brightness(base::dbus::Connection *bus = base::dbus::Connection::session());

    int value() const { return m_value; }
    bool isEnabled() const { return m_enabled; }
    void setValue(int value);

    base::Signal<> valueChanged;
    base::Signal<> enabledChanged;

private:
    void refresh();

    base::dbus::Proxy m_proxy;
    base::dbus::Match m_changed;
    base::dbus::NameWatch m_owner;
    PendingCalls m_calls;
    int m_value = 0;
    bool m_enabled = false;
};

// The keyboard backlight through UPower's KbdBacklight; unavailable while
// UPower has none, when the keys do nothing.
class KeyboardBrightness
{
public:
    explicit KeyboardBrightness(base::dbus::Connection *bus = base::dbus::Connection::system());

    int value() const { return m_value; }
    int maximum() const { return m_maximum; }
    bool isAvailable() const { return m_maximum > 0; }
    void setValue(int value);

    base::Signal<> valueChanged;
    base::Signal<> maximumChanged;
    // The firmware's own hotkey changed it ("internal" source).
    base::Signal<> changedByFirmware;

private:
    void refresh();
    void onBrightnessChanged(int value, const std::string &source);

    base::dbus::Proxy m_proxy;
    base::dbus::Match m_changed;
    PendingCalls m_calls;
    int m_value = 0;
    int m_maximum = 0;
};

} // namespace Lingmo::Shell

