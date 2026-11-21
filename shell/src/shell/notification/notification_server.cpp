// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/notification/notification_server.h"

#include <algorithm>
#include <chrono>

#include "base/logging.h"
#include "base/strings.h"
#include "base/url.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/notification/notification_settings.h"

namespace Lingmo::Shell {

namespace dbus = base::dbus;

static const base::LogCategory lingmoNotifications("lingmo.shell.notifications");

static constexpr const char *kService = "org.freedesktop.Notifications";
static constexpr const char *kPath = "/org/freedesktop/Notifications";
static constexpr const char *kInterface = "org.freedesktop.Notifications";
// A repeat counts only within this time of the first.
static constexpr int64_t kRepeatWindowMs = 1000;

static int64_t nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
}

static std::string localPath(const std::string &pathOrUrl)
{
    if (pathOrUrl.starts_with("file:"))
        return base::url::toLocalFile(pathOrUrl).value_or(std::string());
    return pathOrUrl;
}

// An int of whatever integer type a sender used: "urgency" is a
// byte by the spec, but some send an int.
static int32_t intHint(const dbus::VariantMap &hints, const char *key, int32_t fallback)
{
    const auto it = hints.find(key);
    if (it == hints.end())
        return fallback;
    const dbus::Variant &value = it->second;
    if (auto v = value.get<uint8_t>())
        return *v;
    if (auto v = value.get<int16_t>())
        return *v;
    if (auto v = value.get<uint16_t>())
        return *v;
    if (auto v = value.get<int32_t>())
        return *v;
    if (auto v = value.get<uint32_t>())
        return int32_t(*v);
    if (auto v = value.get<int64_t>())
        return int32_t(*v);
    if (auto v = value.get<uint64_t>())
        return int32_t(*v);
    if (auto v = value.get<bool>())
        return *v ? 1 : 0;
    if (auto v = value.get<double>())
        return int32_t(*v);
    // A string that holds a number reads as it, anything else as 0.
    if (auto v = value.get<std::string>())
        return base::strings::toNumber<int32_t>(*v).value_or(0);
    return 0;
}

static std::string stringHint(const dbus::VariantMap &hints, const char *key)
{
    const auto it = hints.find(key);
    return it == hints.end() ? std::string() : it->second.value<std::string>();
}

NotificationServer::NotificationServer(dbus::Connection *bus, NotificationSettings &settings,
                                       Applications &applications, TaskManager *tasks,
                                       std::string imageDirectory)
    : m_bus(bus)
    , m_settings(settings)
    , m_applications(applications)
    , m_tasks(tasks)
    , m_imageDirectory(std::move(imageDirectory))
{
    m_settingsReady = m_settings.ready.connect([this] {
        const std::deque<Pending *> undecided = std::exchange(m_undecided, {});
        for (Pending *pending : undecided)
            decide(pending);
    });
}

NotificationServer::~NotificationServer()
{
    if (m_object && m_bus)
        m_bus->releaseName(kService);
}

bool NotificationServer::registerService()
{
    if (m_object)
        return true;
    if (!m_bus) {
        LM_WARNING(lingmoNotifications) << "Cannot serve notifications: no session bus";
        return false;
    }

    auto object = std::make_unique<dbus::Object>(m_bus, kPath, kInterface);
    object->addMethod("Notify", "susssasa{sv}i", "u", [this](dbus::MethodCall &call) { notify(call); },
                      dbus::ArgumentNames{{"app_name", "replaces_id", "app_icon", "summary", "body", "actions",
                                           "hints", "expire_timeout"},
                                          {"id"}});
    object->addMethod("CloseNotification", "u", "", [this](dbus::MethodCall &call) { closeFromBus(call); },
                      dbus::ArgumentNames{{"id"}, {}});
    object->addMethod(
            "GetCapabilities",
            [] {
                return std::vector<std::string>{"body", "body-hyperlinks", "body-markup", "icon-static", "actions",
                                                "persistence"};
            },
            dbus::ArgumentNames{{}, {"capabilities"}});
    object->addMethod(
            "GetServerInformation",
            [] { return std::tuple<std::string, std::string, std::string, std::string>("Lingmo", "LingmoOS", "1.0", "1.2"); },
            dbus::ArgumentNames{{}, {"name", "vendor", "version", "spec_version"}});
    object->addSignal("ActionInvoked", "us");
    object->addSignal("NotificationClosed", "uu");
    if (!object->publish()) {
        LM_WARNING(lingmoNotifications) << "Cannot export" << kPath;
        return false;
    }
    if (!m_bus->requestName(kService, dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoNotifications) << "Cannot own" << kService << "- another notification server runs";
        return false;
    }
    m_object = std::move(object);
    return true;
}

bool NotificationServer::isOpen(uint32_t id) const
{
    if (m_open.count(id))
        return true;
    // On its way to the screen; it is open unless its application has
    // notifications turned off, which is not known yet.
    const Pending *pending = pendingFor(id);
    return pending && !pending->off && !pending->closeRequested;
}

// A replacement can follow its notification before either is shown; the
// newest one is what the application now refers to.
NotificationServer::Pending *NotificationServer::pendingFor(uint32_t id) const
{
    for (auto it = m_pending.rbegin(); it != m_pending.rend(); ++it) {
        if ((*it)->notification.id == id)
            return it->get();
    }
    return nullptr;
}

bool NotificationServer::isPending(const Pending *pending) const
{
    return std::any_of(m_pending.begin(), m_pending.end(), [pending](const auto &p) { return p.get() == pending; });
}

void NotificationServer::notify(dbus::MethodCall &call)
{
    std::string appName;
    uint32_t replacesId = 0;
    std::string appIcon;
    std::string summary;
    std::string body;
    std::vector<std::string> actions;
    dbus::VariantMap hints;
    int32_t timeout = -1;
    if (!call.read(appName, replacesId, appIcon, summary, body, actions, hints, timeout)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    // An id no longer open is not reused: the counter may hand it to another
    // notification.
    const bool wasReplaced = replacesId > 0 && isOpen(replacesId);

    auto pending = std::make_unique<Pending>();
    Notification &notification = pending->notification;
    notification.created = nowMs();
    notification.service = call.message().sender();
    notification.summary = summary;
    notification.body = body;
    notification.appName = appName;
    notification.appIcon = localPath(appIcon);
    notification.actions = actions;
    notification.timeout = timeout;
    notification.desktopEntry = stringHint(hints, "desktop-entry");
    notification.urgency = intHint(hints, "urgency", 1);
    notification.suppressSound = intHint(hints, "suppress-sound", 0) != 0;
    notification.soundName = stringHint(hints, "sound-name");
    notification.value = intHint(hints, "value", -1);

    // A replacement is an update the application asked for, such as
    // progress, never a duplicate. A repeat counts only while the first is
    // still open and offers the same actions.
    if (!wasReplaced && isOpen(m_last.id) && m_last.appName == notification.appName
        && m_last.summary == notification.summary && m_last.body == notification.body
        && m_last.actions == notification.actions && m_last.created
        && *notification.created - *m_last.created < kRepeatWindowMs) {
        call.reply(m_last.id);
        return;
    }

    uint32_t id = replacesId;
    if (!wasReplaced) {
        id = m_nextId++;
        if (m_nextId == 0)
            m_nextId = 1;
    }
    notification.id = id;
    m_last = notification;
    pending->wasReplaced = wasReplaced;
    notification.suppressSound = notification.suppressSound || wasReplaced;

    // "image-data" wins over "image-path"; the underscored names are from
    // older revisions of the spec that some applications still send.
    for (const char *key : {"image-data", "image_data", "icon_data"}) {
        const auto it = hints.find(key);
        if (it != hints.end()) {
            pending->imageData = ImageHint::fromVariant(it->second);
            // A hint that is there but broken still wins, as an empty picture.
            if (!pending->imageData)
                pending->imageData = ImageHint();
            break;
        }
    }
    if (!pending->imageData) {
        std::string path = stringHint(hints, "image-path");
        if (path.empty())
            path = stringHint(hints, "image_path");
        pending->imagePath = localPath(path);
    }

    // The id is handed out whatever follows, so the application can replace
    // or close what it thinks it sent.
    call.reply(id);

    Pending *raw = pending.get();
    m_pending.push_back(std::move(pending));
    lookUpSender(raw);
}

void NotificationServer::lookUpSender(Pending *pending)
{
    const std::string sender = pending->notification.service;
    if (!m_bus || sender.empty() || !m_tasks) {
        decide(pending);
        return;
    }

    const dbus::Proxy bus(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
    m_calls.track([&](std::function<void()> finished) {
        return bus.callAsync(
                "GetConnectionUnixProcessID",
                [this, pending, finished](const dbus::Reply &reply) {
                    // The pending notification lives until it is flushed,
                    // which waits for this.
                    if (isPending(pending)) {
                        const uint32_t pid = reply.value<uint32_t>().value_or(0);
                        Notification &notification = pending->notification;
                        const DesktopEntryPtr entry = visibleOwner(
                                m_tasks->resolveProcess(notification.desktopEntry, pid, notification.appName));
                        if (entry) {
                            notification.desktopEntry = entry->id;
                            if (notification.appName.empty())
                                notification.appName = entry->name;
                        }
                        pending->entry = entry;
                        decide(pending);
                    }
                    finished();
                },
                sender);
    });
}

// A NoDisplay helper that shares the program, such as org.foo.App.kwin,
// stands in for the installed org.foo.App, whose name and icon the user knows.
DesktopEntryPtr NotificationServer::visibleOwner(DesktopEntryPtr entry) const
{
    if (!entry || !entry->noDisplay)
        return entry;

    std::string id = entry->id;
    for (size_t dot = id.rfind('.'); dot != std::string::npos && dot > 0; dot = id.rfind('.')) {
        id.resize(dot);
        DesktopEntryPtr owner = m_applications.byId(id);
        if (owner && owner->shouldShow())
            return owner;
    }
    return entry;
}

void NotificationServer::decide(Pending *pending)
{
    if (!m_settings.isReady()) {
        m_undecided.push_back(pending);
        return;
    }

    Notification &notification = pending->notification;
    const std::string applicationKey = notification.applicationKey();
    m_settings.noteApplication(applicationKey, notification.appName, notification.appIcon);

    if (m_settings.applicationMode(applicationKey) == NotificationSettings::Off) {
        pending->off = true;
        pending->resolved = true;
        flush();
        return;
    }

    if (pending->imageData) {
        pending->imageJob = runOnFileThread(
                [image = std::move(*pending->imageData), directory = m_imageDirectory] {
                    return image.pixels.empty() ? std::string()
                                                : notification_images::save(directory, image.toBitmap());
                },
                [this, pending](std::string path) {
                    if (!isPending(pending))
                        return;
                    pending->notification.image = path;
                    finishImages(pending);
                });
        pending->imageData.reset();
        return;
    }

    const std::string &path = pending->imagePath;
    if (path.starts_with('/'))
        notification.image = path;
    else if (!path.empty() && notification.appIcon.empty())
        notification.appIcon = path; // An icon name rather than a picture.
    finishImages(pending);
}

void NotificationServer::finishImages(Pending *pending)
{
    Notification &notification = pending->notification;
    if (pending->entry && notification.appIcon.empty())
        notification.appIcon = pending->entry->icon;

    // With no icon of its own the picture stands in for it.
    if (notification.appIcon.empty() && !notification.image.empty()) {
        notification.appIcon = notification.image;
        notification.image.clear();
    }

    pending->resolved = true;
    flush();
}

void NotificationServer::flush()
{
    while (!m_pending.empty() && m_pending.front()->resolved) {
        std::unique_ptr<Pending> pending = std::move(m_pending.front());
        m_pending.pop_front();
        if (pending->off)
            continue;

        Notification &notification = pending->notification;
        notification.suppressSound = notification.suppressSound || pending->closeRequested;
        const uint32_t id = notification.id;
        const bool replaced = pending->wasReplaced && m_open.count(id);
        m_open.insert(id);
        if (replaced) {
            notification.updated = nowMs();
            notificationReplaced(id, notification);
        } else {
            notificationAdded(notification);
        }
        if (pending->closeRequested)
            closeNotification(id, CloseReason::Revoked);
    }
}

void NotificationServer::closeFromBus(dbus::MethodCall &call)
{
    uint32_t id = 0;
    call.read(id);
    if (Pending *pending = pendingFor(id); pending && !pending->off && !pending->closeRequested) {
        pending->closeRequested = true;
        call.reply();
        return;
    }
    // The spec answers an id that is no longer open with an error, and no
    // signal.
    if (!m_open.count(id)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "No open notification " + std::to_string(id));
        return;
    }
    closeNotification(id, CloseReason::Revoked);
    call.reply();
}

void NotificationServer::invokeAction(uint32_t id, const std::string &actionKey)
{
    if (m_object)
        m_object->emitSignal("ActionInvoked", id, actionKey);
}

void NotificationServer::closeNotification(uint32_t id, CloseReason reason)
{
    m_open.erase(id);
    notificationRemoved(id, reason);
    if (m_object)
        m_object->emitSignal("NotificationClosed", id, uint32_t(reason));
}

} // namespace Lingmo::Shell
