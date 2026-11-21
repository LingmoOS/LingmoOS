// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "base/dbus.h"
#include "base/elapsed_timer.h"
#include "base/signal.h"
#include "base/timer.h"
#include "lingmo/display/display_client.h"

class BrightnessManager;
class WaylandIdle;

// Turns the display off when the user is away, then sleeps; handles the lid.
class DisplayPolicy
{
public:
    // The screen saver on `session`, logind on `system`; `displayService`
    // names the compositor's display service, which tests run themselves.
    // Without `idle`, as in tests, the caller reports idleness itself.
    DisplayPolicy(WaylandIdle *idle, BrightnessManager *brightness,
                  Lingmo::Base::DBus::Connection *session, Lingmo::Base::DBus::Connection *system,
                  const std::string &displayService = "org.lingmo.DisplayConfig");
    ~DisplayPolicy();

    DisplayPolicy(const DisplayPolicy &) = delete;
    DisplayPolicy &operator=(const DisplayPolicy &) = delete;

    // The user has been away for `seconds`, one of the watched durations.
    void idleFor(int seconds);
    // The user came back after at least one idleFor().
    void userReturned();

    void setPolicy(int timeout, int suspendDelay);
    void setLidPresent(bool present);
    void handleLidClosed();
    void handleLidOpened();

private:
    int watchIdle(int seconds);
    void unwatchIdle(int identifier);

    void onPrepareForSleep(bool sleeping);
    void onScreenSaverActiveChanged(bool active);
    void onLogindPropertiesChanged(const Lingmo::Base::DBus::VariantMap &changedProperties,
                                   const std::vector<std::string> &invalidatedProperties);
    void onSuspendTimerTimeout();
    void onDisplayStateChanged();

    bool lockSession();
    void trySuspend();
    void fade(int divisor);
    void turnDisplayOff();
    void restoreDisplay();
    void startSuspendCountdown();
    void stopSuspendCountdown();
    void turnDisplayOffWhenIdle();
    void sleepWithLidClosed(bool waitForLock);
    bool idleSleepApplies() const;
    void updateLidSwitchInhibitor();
    bool hasNativeDisplayPower() const;
    bool setDisplayPower(bool on);
    void requestDockedLid(bool closed);
    void finishDockedLid(bool closed, bool succeeded, Lingmo::Display::ErrorCode error);
    int screenBrightness() const;
    void setScreenBrightness(int value);

    WaylandIdle *m_idle;
    BrightnessManager *m_brightness;
    Lingmo::Base::DBus::Proxy m_screenSaver;
    Lingmo::Base::DBus::Proxy m_logind;
    Lingmo::Display::DisplayClient m_display;
    std::vector<Lingmo::Base::DBus::Match> m_matches;
    std::vector<Lingmo::Base::Subscription> m_idleSubscriptions;
    Lingmo::Base::Subscription m_displayStateChanged;
    Lingmo::Base::DBus::PendingCall m_suspendCall;
    Lingmo::Base::DBus::PendingCall m_inhibitCall;
    std::set<int> m_idleIds;
    // One of the watched durations has passed since the user last came back.
    bool m_away = false;
    int m_timeout = 0;
    int m_suspendDelay = 900;
    // The brightness to return to; -1 while nothing has lowered it.
    int m_savedBrightness = -1;
    bool m_dimmed = false;
    bool m_displayPoweredOff = false;
    bool m_displayOff = false;
    bool m_suspendRequested = false;
    bool m_suspendDue = false;
    uint64_t m_suspendSerial = 0;
    bool m_lidClosed = false;
    bool m_lidPresent = false;
    bool m_lidDisplayOff = false;
    // Closed with no display in use: logind handles the lid until it opens.
    bool m_lidSleeping = false;
    bool m_locked = false;
    // The user has been away for the locked timeout since their last input.
    bool m_lockedIdle = false;
    int m_lockedIdleTimeout = -1;
    std::vector<int> m_policyIdleTimeouts;
    bool m_dockedLidPending = false;
    bool m_dockedLidActive = false;
    bool m_dockedLidRecheck = false;
    int m_dockedLidRetries = 0;
    uint64_t m_dockedLidSerial = 0;
    Lingmo::Base::Timer m_dockedLidRetryTimer;
    std::tuple<std::string, uint64_t, uint64_t> m_displayTopology;

    Lingmo::Base::DBus::UnixFd m_lidSwitchInhibitor;
    bool m_lidSwitchInhibiting = false;

    Lingmo::Base::Timer m_suspendTimer;
    Lingmo::Base::Timer m_wakeTimer;
    Lingmo::Base::ElapsedTimer m_displayOffTimer;
};

