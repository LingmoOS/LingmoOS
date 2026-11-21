
#include <memory>
#include <string>
#pragma once
#include <vector>

#include "base/dbus.h"
#include "base/settings.h"
#include "base/signal.h"

class BrightnessManager;
class DisplayPolicy;
class PowerSupply;
class WaylandIdle;

class PowerManager
{
public:
    PowerManager(PowerSupply *powerSupply, BrightnessManager *brightness, WaylandIdle *idle,
                 Lingmo::Base::Settings &settings, Lingmo::Base::DBus::Connection *session,
                 Lingmo::Base::DBus::Connection *system);
    ~PowerManager();

    PowerManager(const PowerManager &) = delete;
    PowerManager &operator=(const PowerManager &) = delete;

    // org.lingmo.Services.Power at /org/lingmo/Services/Power.
    bool publish();

    void setBatteryScreenOff(int timeout);
    void setACScreenOff(int timeout);
    void setACPreventSleep(bool prevent);
    void setDimDisplayTimeout(int timeout);

private:
    void onPowerSourceChanged();
    void onLidChanged();
    void loadSettings();
    void applyPolicy();
    template<typename T>
    void writePowerSetting(const std::string &key, const T &value);

    Lingmo::Base::Settings &m_settings;
    Lingmo::Base::DBus::Connection *m_session;
    PowerSupply *m_powerSupply;
    std::unique_ptr<DisplayPolicy> m_displayPolicy;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_object;
    std::vector<Lingmo::Base::Subscription> m_subscriptions;

    int m_batteryScreenOff = 300;
    int m_acScreenOff = 1200;
    bool m_acPreventSleep = false;
    bool m_onBattery = false;
};

