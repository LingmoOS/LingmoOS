// SPDX-FileCopyrightText: 2026 Lingmo OS Team
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lingmo/update/update_status.h"

#include <cstdint>

#include "base/event_loop.h"

using namespace Lingmo::Base;

namespace Lingmo::update {

static constexpr const char* kService   = "org.lingmo.Services";
static constexpr const char* kPath      = "/org/lingmo/Services/Update";
static constexpr const char* kInterface = "org.lingmo.Services.Update";

std::shared_ptr<UpdateStatus> UpdateStatus::shared()
{
    thread_local std::weak_ptr<UpdateStatus> instance;
    thread_local base::EventLoop* instanceLoop = nullptr;

    base::EventLoop* loop = base::EventLoop::current();
    std::shared_ptr<UpdateStatus> status = instance.lock();
    if (!status || instanceLoop != loop) {
        status = std::make_shared<UpdateStatus>();
        instance = status;
        instanceLoop = loop;
    }
    return status;
}

UpdateStatus::UpdateStatus()
    : m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    m_service.setTimeout(std::chrono::seconds(10));
    dbus::Connection* bus = m_service.connection();
    if (!bus)
        return;

    // Read synchronously so the first frame already has the values.
    if (const dbus::Reply reply = m_service.getAll(); !reply.isError()) {
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available  = true;
        // Prime the cached progress fields.
        m_upgradeProgress = value<int32_t>("upgradeProgress", -1);
        m_upgradePhase    = value<int32_t>("upgradePhase", 0);
    }

    // stateChanged and property changes trigger a full reload.
    for (const char* member : {"stateChanged"})
        m_matches.push_back(m_service.onSignal(member, [this](const dbus::Message&) { reload(); }));

    // upgradeProgress carries two ints (phase, percent) and is handled
    // separately to avoid a round-trip for every progress tick.
    m_matches.push_back(m_service.onSignal("upgradeProgress", [this](const dbus::Message& msg) {
        int32_t phase = 0, percent = 0;
        msg.read(phase, percent);
        m_upgradePhase    = phase;
        m_upgradeProgress = percent;
        upgradeProgressChanged.emit();
    }));

    m_matches.push_back(m_service.onPropertiesChanged([this](const dbus::VariantMap&, const std::vector<std::string>&) { reload(); }));

    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        const bool available = !owner.empty();
        if (available) {
            reload();
        } else {
            m_reload = {}; m_request = {};
            m_requestPending = false;
            m_available = false;
            m_properties.clear();
            m_upgradeProgress = -1;
            m_upgradePhase    = 0;
            changed.emit();
        }
    });
}

UpdateStatus::~UpdateStatus() = default;

void UpdateStatus::reload()
{
    m_reload = m_service.getAllAsync([this](const dbus::Reply& reply) {
        if (reply.isError()) {
            m_available = false;
            m_requestError = reply.error().message;
            changed.emit();
            return;
        }
        m_properties = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
        m_available  = true;
        // Keep cached progress in sync with the full properties map.
        m_upgradeProgress = value<int32_t>("upgradeProgress", -1);
        m_upgradePhase    = value<int32_t>("upgradePhase", 0);
        changed.emit();
    });
}

template<typename T>
T UpdateStatus::value(const char* name, const T& fallback) const
{
    const auto it = m_properties.find(name);
    return it == m_properties.end() ? fallback : it->second.value<T>(fallback);
}

// ── property getters ──────────────────────────────────────────────────────────

State UpdateStatus::state() const
{
    const int32_t v = value<int32_t>("state", 0);
    if (v >= 0 && v <= 7) return static_cast<State>(v);
    return State::Idle;
}

Channel UpdateStatus::channel() const
{
    const int32_t v = value<int32_t>("channel", 0);
    if (v >= 0 && v <= 2) return static_cast<Channel>(v);
    return Channel::Stable;
}

bool UpdateStatus::autoCheck() const { return value<bool>("autoCheck", true); }

int UpdateStatus::checkMode() const
{
    const int32_t v = value<int32_t>("checkMode", 0);
    return v == 1 ? 1 : 0;
}

int UpdateStatus::checkInterval() const
{
    return int(value<int32_t>("checkInterval", 360));
}

int UpdateStatus::checkHour() const
{
    const int32_t v = value<int32_t>("checkHour", 3);
    return v < 0 || v > 23 ? 3 : int(v);
}

int UpdateStatus::checkMinute() const
{
    const int32_t v = value<int32_t>("checkMinute", 0);
    return v < 0 || v > 59 ? 0 : int(v);
}

std::string UpdateStatus::newVersion() const    { return value<std::string>("newVersion", {}); }
int64_t     UpdateStatus::newBuild() const      { return value<int64_t>("newBuild", 0); }
bool        UpdateStatus::mandatory() const     { return value<bool>("mandatory", false); }
std::string UpdateStatus::releaseNotes() const  { return value<std::string>("releaseNotes", {}); }
std::string UpdateStatus::releaseDate() const   { return value<std::string>("releaseDate", {}); }
std::string UpdateStatus::changelogText() const { return value<std::string>("changelogText", {}); }
int         UpdateStatus::upgradableCount() const { return int(value<int32_t>("upgradableCount", 0)); }
std::string UpdateStatus::upgradablePackages() const { return value<std::string>("upgradablePackages", {}); }
int64_t     UpdateStatus::lastCheckTime() const { return value<int64_t>("lastCheckTime", 0); }
std::string UpdateStatus::errorLog() const      { return value<std::string>("errorLog", {}); }

// ── methods ───────────────────────────────────────────────────────────────────

void UpdateStatus::beginRequest()
{
    m_requestError.clear();
    m_requestPending = true;
    changed.emit();
}

void UpdateStatus::finishRequest(const dbus::Reply& reply)
{
    m_requestPending = false;
    if (reply.isError()) m_requestError = reply.error().message;
    changed.emit();
    if (!reply.isError()) reload();
}

void UpdateStatus::checkForUpdate()
{
    if (!m_available || m_requestPending) return;
    beginRequest();
    m_request = m_service.callAsync("checkForUpdate", [this](const dbus::Reply& reply) { finishRequest(reply); });
}

void UpdateStatus::startUpgrade()
{
    if (!m_available || m_requestPending) return;
    beginRequest();
    m_request = m_service.callAsync("startUpgrade", [this](const dbus::Reply& reply) { finishRequest(reply); });
}

void UpdateStatus::setChannel(int channel)
{
    if (!m_available || m_requestPending) return;
    beginRequest();
    m_request = m_service.callAsync("setChannel", [this](const dbus::Reply& reply) { finishRequest(reply); }, int32_t(channel));
}

void UpdateStatus::setAutoCheck(bool enabled)
{
    if (!m_available || m_requestPending) return;
    beginRequest();
    m_request = m_service.callAsync("setAutoCheck",
                                    [this](const dbus::Reply& reply) { finishRequest(reply); },
                                    enabled);
}

void UpdateStatus::setCheckSchedule(int mode, int minutes)
{
    if (!m_available || m_requestPending) return;
    beginRequest();
    m_request = m_service.callAsync("setCheckSchedule",
                                    [this](const dbus::Reply& reply) { finishRequest(reply); },
                                    int32_t(mode), int32_t(minutes));
}

void UpdateStatus::setCheckTime(int hour, int minute)
{
    if (!m_available || m_requestPending) return;
    beginRequest();
    m_request = m_service.callAsync("setCheckTime",
                                    [this](const dbus::Reply& reply) { finishRequest(reply); },
                                    int32_t(hour), int32_t(minute));
}

} // namespace Lingmo::update
