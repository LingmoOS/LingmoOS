// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/battery/battery.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "base/file.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "lingmo/upower/battery_status.h"
#include "lingmo/upower/device.h"
#include "pages/battery/timeformat.h"
#include "window/options.h"

using namespace Lingmo;
using namespace Lingmo::base;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

static int64_t now()
{
    return int64_t(std::time(nullptr));
}

Battery::Battery()
    : m_mock(options().mockBattery)
{
    if (m_mock) {
        m_udi = "mock-battery";
        m_chargeLimitSupported = true;
        m_chargeLimitStart = 75;
        m_chargeLimitEnd = 80;
        return;
    }

    m_status = upower::BatteryStatus::shared();
    m_subscriptions.push_back(m_status->readyChanged.connect([this] { readyChanged.emit(); }));
    m_subscriptions.push_back(m_status->availableChanged.connect([this] { availableChanged.emit(); }));
    m_subscriptions.push_back(m_status->capacityChanged.connect([this] { capacityChanged.emit(); }));
    m_subscriptions.push_back(m_status->lastChargedChanged.connect([this] { lastChargedChanged.emit(); }));
    m_subscriptions.push_back(m_status->udiChanged.connect([this] { watchDevice(); }));

    m_recheck.setSingleShot(true);
    m_recheck.onTimeout = [this] { readKernelChargeLimit(); };

    watchDevice();
}

Battery::~Battery() = default;

void Battery::watchDevice()
{
    if (m_status->udi() == m_udi)
        return;

    m_deviceSubscription = {};
    m_device.reset();
    m_udi = m_status->udi();

    if (!m_udi.empty()) {
        m_device = std::make_unique<upower::Device>(m_udi);
        m_deviceSubscription = m_device->propertiesChanged.connect([this](const dbus::VariantMap& properties) {
            const auto nativePath = properties.find("NativePath");
            if (nativePath != properties.end()) {
                const std::string name = nativePath->second.value<std::string>();
                if (!name.empty())
                    m_batteryPath = "/sys/class/power_supply/" + name + "/";
            }
            updateChargeLimit(properties);
            // UPower updates the battery every little while; a cheap moment to look again.
            readKernelChargeLimit();
        });
    }

    udiChanged.emit();
}

bool Battery::isReady() const
{
    return m_mock || m_status->isReady();
}

bool Battery::isAvailable() const
{
    return m_mock || m_status->isAvailable();
}

int Battery::lastChargedPercent() const
{
    return m_mock ? 100 : m_status->lastChargedPercent();
}

int Battery::capacity() const
{
    return m_mock ? 94 : m_status->capacity();
}

std::string Battery::lastChargedTime() const
{
    const int64_t secs = m_mock ? now() - 2 * 3600 : m_status->lastChargedSecs();

    if (secs <= 0)
        return {};

    // A date and time, as the file manager shows them; the service's own
    // "13 h 15 min ago" is worked out once and goes stale on an open page.
    const int64_t days = timeformat::daysBetween(secs, now());

    if (days == 0)
        return format(tr("Today at {time}"), {{"time", timeformat::shortTime(secs)}});
    if (days == 1)
        return format(tr("Yesterday at {time}"), {{"time", timeformat::shortTime(secs)}});

    return timeformat::shortDateTime(secs);
}

bool Battery::showPercent() const
{
    if (m_mock)
        return true;

    Settings settings(paths::lingmoConfig("shell"));
    return settings.boolValue("Statusbar/BatteryPercentage", false);
}

void Battery::setPercentEnabled(bool enabled)
{
    if (m_mock)
        return;

    dbus::Proxy(dbus::Connection::session(), "org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar")
            .send("setBatteryPercentage", enabled);
}

bool Battery::chargeLimitHeld() const
{
    if (!m_chargeLimitEnabled || m_kernelChargeLimit < 0)
        return true;

    return m_kernelChargeLimit == m_chargeLimitEnd;
}

void Battery::setChargeLimitEnabled(bool enabled)
{
    if (m_mock) {
        m_chargeLimitEnabled = enabled;
        chargeLimitChanged.emit();
        return;
    }

    if (!m_device)
        return;

    m_device->enableChargeThreshold(enabled, [this](const dbus::Error& error) {
        if (error.isSet()) {
            fprintf(stderr, "Failed to change the charge limit: %s\n", error.message.c_str());
            // Puts a switch showing chargeLimitEnabled() back.
            chargeLimitChanged.emit();
        }
    });
}

void Battery::readKernelChargeLimit()
{
    if (m_batteryPath.empty())
        return;

    int limit = -1;
    if (const auto contents = file::read(m_batteryPath + "charge_control_end_threshold"))
        limit = std::atoi(contents->c_str());

    if (limit == m_kernelChargeLimit)
        return;

    m_kernelChargeLimit = limit;
    chargeLimitChanged.emit();
}

// UPower before 1.90 has none of these properties, which leaves the limit unsupported.
void Battery::updateChargeLimit(const dbus::VariantMap& properties)
{
    const auto flag = [&properties](const char* name, bool fallback) {
        const auto it = properties.find(name);
        return it == properties.end() ? fallback : it->second.value<bool>(fallback);
    };
    const auto number = [&properties](const char* name, int fallback) {
        const auto it = properties.find(name);
        return it == properties.end() ? fallback : int(upower::integerValue(it->second, fallback));
    };

    const bool supported = flag("ChargeThresholdSupported", m_chargeLimitSupported);
    const bool enabled = flag("ChargeThresholdEnabled", m_chargeLimitEnabled);
    const int start = number("ChargeStartThreshold", m_chargeLimitStart);
    const int end = number("ChargeEndThreshold", m_chargeLimitEnd);

    if (supported == m_chargeLimitSupported && enabled == m_chargeLimitEnabled && start == m_chargeLimitStart
        && end == m_chargeLimitEnd)
        return;

    const bool turnedOn = enabled && !m_chargeLimitEnabled;

    m_chargeLimitSupported = supported;
    m_chargeLimitEnabled = enabled;
    m_chargeLimitStart = start;
    m_chargeLimitEnd = end;
    chargeLimitChanged.emit();

    // A controller that rejects the thresholds resets them within seconds,
    // without UPower or the kernel announcing it.
    if (turnedOn)
        m_recheck.start(std::chrono::seconds(5));
}

} // namespace settings
