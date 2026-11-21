// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clockcore.h"
#include "clockstore.h"
#include <base/dbus.h>
#include <base/event_loop.h>
#include <base/i18n.h>
#include <base/locale.h>
#include <base/session_language.h>
#include <cerrno>
#include <cstdint>
#include <memory>
#include <spawn.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

extern char** environ;

namespace dbus = Lingmo::base::dbus;
using Lingmo::I18n::tr;
namespace {
constexpr auto kService = "org.lingmo.Clock";
constexpr auto kPath = "/org/lingmo/Clock";
constexpr auto kInterface = "org.lingmo.Clock";
constexpr int64_t kCatchUpSeconds = 300;

class Scheduler {
  public:
    Scheduler(Lingmo::base::EventLoop& loop, Clock::Store& store, dbus::Connection* bus, dbus::Connection* systemBus)
        : m_loop(loop), m_store(store), m_bus(bus), m_fd(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC)) {
        if (m_fd >= 0)
            m_watch = m_loop.watch(m_fd, EPOLLIN | EPOLLERR, [this](uint32_t) { onTimer(); });
        m_object = std::make_unique<dbus::Object>(bus, kPath, kInterface);
        m_object->addMethod("Reschedule", "", "", [this](dbus::MethodCall& call) {
            call.reply();
            schedule();
        });
        m_object->addMethod("ListAlarms", "", "as", [this](dbus::MethodCall& call) {
            std::vector<std::string> lines;
            for (const auto& a : m_store.alarms())
                lines.push_back(std::to_string(a.id) + "\t" + a.name + "\t" + std::to_string(a.hour) + ":" +
                                std::to_string(a.minute) + "\t" + (a.enabled ? "1" : "0"));
            call.reply(lines);
        });
        m_object->addMethod("SetAlarmEnabled", "xb", "", [this](dbus::MethodCall& call) {
            int64_t id = 0;
            bool enabled = false;
            if (!call.read(id, enabled)) {
                call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Expected alarm id and enabled state");
                return;
            }
            m_store.setAlarmEnabled(id, enabled);
            call.reply();
            schedule();
        });
        m_object->addMethod("RemoveAlarm", "x", "", [this](dbus::MethodCall& call) {
            int64_t id = 0;
            if (!call.read(id)) {
                call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Expected alarm id");
                return;
            }
            m_store.removeAlarm(id);
            call.reply();
            schedule();
        });
        m_object->publish();
        m_timeZoneChanged = systemBus ? systemBus->subscribe({"org.freedesktop.timedate1", "/org/freedesktop/timedate1",
                                                              "", "org.freedesktop.DBus.Properties",
                                                              "PropertiesChanged", "org.freedesktop.timedate1"},
                                                             [this](const dbus::Message&) { schedule(); })
                                      : dbus::Match{};
        m_notificationActions =
            bus ? bus->subscribe(
                      {"org.freedesktop.Notifications", "/org/freedesktop/Notifications", "",
                       "org.freedesktop.Notifications", "ActionInvoked", ""},
                      [this](const dbus::Message& message) {
                          uint32_t id = 0;
                          std::string key;
                          if (!message.read(id, key) || key != "snooze")
                              return;
                          auto found = m_shown.find(id);
                          if (found == m_shown.end())
                              return;
                          Clock::Alarm snoozed = found->second;
                          snoozed.id = 0;
                          snoozed.name += std::string(" - ") + tr("Snoozed");
                          snoozed.recurring = false;
                          snoozed.weekdays = 0;
                          snoozed.nextOccurrenceUtc =
                              std::chrono::duration_cast<std::chrono::seconds>(
                                  (std::chrono::system_clock::now() + std::chrono::minutes(5)).time_since_epoch())
                                  .count();
                          m_store.saveAlarm(snoozed);
                          m_shown.erase(found);
                          schedule();
                      })
                : dbus::Match{};
        m_notificationClosed = bus ? bus->subscribe({"org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                                                     "", "org.freedesktop.Notifications", "NotificationClosed", ""},
                                                    [this](const dbus::Message& message) {
                                                        uint32_t id = 0;
                                                        uint32_t reason = 0;
                                                        if (message.read(id, reason))
                                                            m_shown.erase(id);
                                                    })
                                   : dbus::Match{};
        m_notificationActionDismissed =
            bus ? bus->subscribe({"org.freedesktop.Notifications", "/org/freedesktop/Notifications", "",
                                  "org.freedesktop.Notifications", "ActionInvoked", ""},
                                 [this](const dbus::Message& message) {
                                     uint32_t id = 0;
                                     std::string key;
                                     if (message.read(id, key) && key == "dismiss")
                                         m_shown.erase(id);
                                 })
                : dbus::Match{};
        schedule();
    }
    ~Scheduler() {
        *m_alive = false;
        m_timeZoneChanged.reset();
        m_notificationActions.reset();
        m_notificationClosed.reset();
        m_notificationActionDismissed.reset();
        m_object.reset();
        m_pending.clear();
        if (m_watch)
            m_loop.unwatch(m_watch);
        if (m_fd >= 0)
            close(m_fd);
    }
    void schedule() {
        if (m_fd < 0)
            return;
        itimerspec spec{};
        int64_t nearest = 0;
        const auto now = std::chrono::system_clock::now();
        const auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        for (auto alarm : m_store.alarms()) {
            if (!alarm.enabled)
                continue;
            int64_t occurrence = alarm.nextOccurrenceUtc;
            if (alarm.recurring && (occurrence == 0 || occurrence < nowSec - kCatchUpSeconds)) {
                const int64_t updated = Clock::nextAlarmOccurrenceUtc(alarm.hour, alarm.minute, alarm.weekdays, now);
                if (updated > 0 && updated != occurrence) {
                    alarm.nextOccurrenceUtc = updated;
                    m_store.saveAlarm(alarm);
                }
                occurrence = updated;
            }
            if (occurrence > 0 && (!nearest || occurrence < nearest))
                nearest = occurrence;
        }
        const int64_t recheck = nowSec + 3600;
        if (!nearest || recheck < nearest)
            nearest = recheck;
        spec.it_value.tv_sec = nearest;
        timerfd_settime(m_fd, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &spec, nullptr);
    }

  private:
    void notify(const Clock::Alarm& alarm) {
        if (!m_bus)
            return;
        auto call = m_bus->newMethodCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications",
                                         "org.freedesktop.Notifications", "Notify");
        const std::string body = alarm.name.empty() ? tr("Alarm") : alarm.name;
        call.append(std::string("Lingmo Clock"), uint32_t(0), std::string("alarm-clock"), std::string(tr("Alarm")),
                    body,
                    std::vector<std::string>{std::string("snooze"), std::string(tr("Snooze")), std::string("dismiss"),
                                             std::string(tr("Dismiss"))},
                    dbus::VariantMap{}, int32_t(0));
        auto alive = m_alive;
        m_pending.push_back(m_bus->callAsync(call, [this, alive, alarm](const dbus::Reply& reply) {
            if (!*alive)
                return;
            if (auto id = reply.value<uint32_t>())
                m_shown[*id] = alarm;
        }));
        playSound(alarm.sound);
    }
    void playSound(const std::string& sound) {
        const std::string file = sound.empty() ? "/usr/share/sounds/freedesktop/stereo/alarm-clock-elapsed.oga" : sound;
        pid_t pid = 0;
        char* argv[] = {const_cast<char*>("paplay"), const_cast<char*>(file.c_str()), nullptr};
        if (posix_spawnp(&pid, "paplay", nullptr, nullptr, argv, environ) == 0) {
            std::thread([pid] {
                int status = 0;
                while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
                }
            }).detach();
        }
    }
    void onTimer() {
        uint64_t expirations = 0;
        const ssize_t n = read(m_fd, &expirations, sizeof(expirations));
        if (n < 0 && errno != ECANCELED && errno != EAGAIN)
            return;
        const auto now = std::chrono::system_clock::now();
        const int64_t nowSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        for (auto alarm : m_store.alarms()) {
            if (!alarm.enabled)
                continue;
            int64_t occurrence = alarm.nextOccurrenceUtc;
            if (alarm.recurring && (occurrence == 0 || occurrence < nowSec - kCatchUpSeconds))
                occurrence = Clock::nextAlarmOccurrenceUtc(alarm.hour, alarm.minute, alarm.weekdays,
                                                           now - std::chrono::seconds(kCatchUpSeconds));
            if (occurrence > 0 && occurrence <= nowSec + kCatchUpSeconds) {
                notify(alarm);
                if (!alarm.recurring)
                    m_store.removeAlarm(alarm.id);
                else {
                    alarm.nextOccurrenceUtc =
                        Clock::nextAlarmOccurrenceUtc(alarm.hour, alarm.minute, alarm.weekdays, now);
                    m_store.saveAlarm(alarm);
                }
            }
        }
        schedule();
    }
    Lingmo::base::EventLoop& m_loop;
    Clock::Store& m_store;
    dbus::Connection* m_bus;
    int m_fd = -1;
    int m_watch = 0;
    std::unique_ptr<dbus::Object> m_object;
    dbus::Match m_timeZoneChanged;
    dbus::Match m_notificationActions;
    dbus::Match m_notificationClosed;
    dbus::Match m_notificationActionDismissed;
    std::map<uint32_t, Clock::Alarm> m_shown;
    std::vector<dbus::PendingCall> m_pending;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
} // namespace
int main() {
    Lingmo::base::Locale::setProcessLocale();
    Lingmo::i18n::setDomain("lingmo-clock");
    Lingmo::base::EventLoop loop;
    auto* bus = dbus::Connection::session(&loop);
    if (!bus || !bus->requestName(kService))
        return 1;
    auto* systemBus = dbus::Connection::system(&loop);
    Lingmo::base::SessionLanguage language(bus);
    Clock::Store store;
    Scheduler scheduler(loop, store, bus, systemBus);
    return loop.run();
}
