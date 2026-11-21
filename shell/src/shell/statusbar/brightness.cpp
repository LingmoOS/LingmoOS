// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/brightness.h"

#include <algorithm>

#include "shell/common/service_owner.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

Brightness::Brightness(dbus::Connection *bus)
    : m_proxy(bus, "org.lingmo.Services", "/org/lingmo/Services/Brightness", "org.lingmo.Services.Brightness")
{
    // Keys, dimming and the sliders all change it.
    m_changed = m_proxy.onSignal("brightnessChanged", [this](const dbus::Message &) { refresh(); });
    m_owner = followServiceOwner(bus, "org.lingmo.Services", [this] { refresh(); });
}

void Brightness::refresh()
{
    m_calls.track([&](std::function<void()> finished) {
        return m_proxy.getAllAsync([this, finished](const dbus::Reply &reply) {
            if (const auto properties = reply.value<dbus::VariantMap>()) {
                const auto find = [&properties](const char *name) {
                    const auto it = properties->find(name);
                    return it == properties->end() ? dbus::Variant() : it->second;
                };
                const int value = find("brightness").value<int32_t>();
                const bool enabled = find("brightnessEnabled").value<bool>();
                if (m_value != value) {
                    m_value = value;
                    valueChanged();
                }
                if (m_enabled != enabled) {
                    m_enabled = enabled;
                    enabledChanged();
                }
            }
            finished();
        });
    });
}

void Brightness::setValue(int value)
{
    m_proxy.send("setValue", int32_t(value));
}

KeyboardBrightness::KeyboardBrightness(dbus::Connection *bus)
    : m_proxy(bus, "org.freedesktop.UPower", "/org/freedesktop/UPower/KbdBacklight",
              "org.freedesktop.UPower.KbdBacklight")
{
    m_changed = m_proxy.onSignal("BrightnessChangedWithSource", [this](const dbus::Message &message) {
        int32_t value = 0;
        std::string source;
        if (message.read(value, source))
            onBrightnessChanged(value, source);
    });
    refresh();
}

void KeyboardBrightness::refresh()
{
    // UPower has no such object without a backlight: the call fails and the
    // keys do nothing.
    m_calls.track([&](std::function<void()> finished) {
        return m_proxy.callAsync("GetMaxBrightness", [this, finished](const dbus::Reply &reply) {
            const int32_t maximum = reply.value<int32_t>().value_or(0);
            if (maximum > 0) {
                m_maximum = maximum;
                maximumChanged();
                m_calls.track([&](std::function<void()> brightnessFinished) {
                    return m_proxy.callAsync("GetBrightness", [this, brightnessFinished](const dbus::Reply &brightness) {
                        if (const auto value = brightness.value<int32_t>())
                            onBrightnessChanged(*value, {});
                        brightnessFinished();
                    });
                });
            }
            finished();
        });
    });
}

void KeyboardBrightness::onBrightnessChanged(int value, const std::string &source)
{
    if (m_value != value) {
        m_value = value;
        valueChanged();
    }
    if (source == "internal")
        changedByFirmware();
}

void KeyboardBrightness::setValue(int value)
{
    if (!isAvailable())
        return;
    m_proxy.send("SetBrightness", int32_t(std::clamp(value, 0, m_maximum)));
}

} // namespace Lingmo::Shell
