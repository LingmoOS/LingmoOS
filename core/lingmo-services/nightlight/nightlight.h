// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"
#include "lingmo/display/display_client.h"
#include "scheduledfeature.h"

// The night light's preferences and schedule; the compositor only animates the
// strength it is sent. Modelled on Ash's NightLightControllerImpl: a change the
// user makes takes a second, a scheduled one a minute.
class NightLight : public ScheduledFeature
{
public:
    // `displayService` names the compositor's display service; tests run one
    // of their own.
    explicit NightLight(Lingmo::Base::Settings &settings,
                        Clock clock = ScheduleUtils::currentDateTime,
                        const std::string &displayService = "org.lingmo.DisplayConfig");
    ~NightLight() override;

    // org.lingmo.Services.NightLight at /org/lingmo/Services/NightLight.
    bool publish(Lingmo::Base::DBus::Connection *bus);

    // The strength, from 0 to 1.
    double temperature() const { return m_temperature; }

    // Whether a display the compositor shows can be tinted.
    bool available() const { return m_available; }

    // Whether the temperature shows while the night light is off, as it does
    // while Settings' slider is held.
    bool previewing() const { return m_previewing; }

    void setTemperature(double temperature);
    // Ends by itself when `caller`, a bus name, leaves the bus.
    void setPreview(bool preview, const std::string &caller = {});

    Lingmo::Base::Signal<double> temperatureChanged;
    Lingmo::Base::Signal<bool> availableChanged;

protected:
    void showEnabled(std::chrono::milliseconds duration) override;

private:
    void endPreview();
    void sendToCompositor(std::chrono::milliseconds duration);
    void updateAvailable();

    Lingmo::Display::DisplayClient m_display;
    Lingmo::Base::DBus::Connection *m_bus = nullptr;
    Lingmo::Base::DBus::NameWatch m_previewWatch;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_object;
    std::vector<Lingmo::Base::Subscription> m_subscriptions;
    double m_temperature = 0.5;
    bool m_available = false;
    bool m_previewing = false;
};

