// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountservice.h"
#include "diskimagebackend.h"
#include "keyring.h"
#include "mtpbackend.h"
#include "mtpmonitor.h"
#include "sftpbackend.h"
#include "smbbackend.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/strings.h"
#include "base/url.h"

#include <climits>
#include <dirent.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

using namespace Lingmo::base;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

static std::unique_ptr<Backend> createBackend(const RemoteUrl &url)
{
    if (url.scheme == "sftp" || url.scheme == "ssh")
        return std::make_unique<SftpBackend>(url);
    if (url.scheme == "smb")
        return std::make_unique<SmbBackend>(url);

    return nullptr;
}

static int defaultPort(const std::string &scheme)
{
    if (scheme == "sftp" || scheme == "ssh")
        return 22;
    if (scheme == "smb")
        return 445;
    return -1;
}

// One mount per account on a server: scheme://user@host[:port].
static RemoteUrl serverKey(const RemoteUrl &url, const std::string &user)
{
    RemoteUrl id;
    id.scheme = url.scheme == "ssh" ? "sftp" : url.scheme;
    id.host = url.host;
    id.user = user;
    if (url.port > 0 && url.port != defaultPort(id.scheme))
        id.port = url.port;
    return id;
}

static std::string serverId(const RemoteUrl &url, const std::string &user)
{
    return serverKey(url, user).toString();
}

static std::string serverName(const RemoteUrl &url, const std::string &user)
{
    const std::string host = url.port > 0 && url.port != defaultPort(url.scheme)
            ? url.host + ':' + std::to_string(url.port)
            : url.host;
    return user.empty() ? host : user + '@' + host;
}

static std::string joined(const std::string &mountPoint, const std::string &path)
{
    if (path.empty() || path == "/")
        return mountPoint;
    return mountPoint + path;
}

static bool exists(const std::string &path)
{
    struct stat st;
    return ::lstat(path.c_str(), &st) == 0;
}

// The disk image a location names: a local path or file: URL of a regular file.
static std::string diskImagePath(const std::string &location)
{
    std::string path = location;
    if (location.starts_with("file:")) {
        const auto local = url::toLocalFile(location);
        if (!local)
            return {};
        path = *local;
    }
    if (!path.starts_with('/'))
        return {};

    char resolved[PATH_MAX];
    struct stat st;
    if (!::realpath(path.c_str(), resolved) || ::stat(resolved, &st) != 0 || !S_ISREG(st.st_mode))
        return {};
    return resolved;
}

static std::string optionString(const dbus::VariantMap &options, const std::string &key)
{
    const auto found = options.find(key);
    return found != options.end() ? found->second.value<std::string>() : std::string();
}

static bool optionBool(const dbus::VariantMap &options, const std::string &key)
{
    const auto found = options.find(key);
    return found != options.end() && found->second.value<bool>();
}

MountService::MountService(dbus::Connection *bus)
    : m_bus(bus)
    , m_object(bus, "/org/lingmo/Mounts1", "org.lingmo.Mounts1")
    , m_root(paths::runtimeDirectory() + "/mounts")
{
    // Mounts a crashed run left behind answer nothing; detach them.
    if (DIR *root = ::opendir(m_root.c_str())) {
        std::vector<std::string> names;
        while (dirent *entry = ::readdir(root)) {
            const std::string name = entry->d_name;
            if (name != "." && name != "..")
                names.push_back(name);
        }
        ::closedir(root);

        for (const std::string &name : names) {
            const std::string path = m_root + '/' + name;
            Process fusermount;
            if (fusermount.start("fusermount3", {"-u", "-z", path}))
                (void)fusermount.waitForFinished(std::chrono::seconds(30));
            ::rmdir(path.c_str());
        }
    }

    m_monitor = std::make_unique<MtpMonitor>();
    m_added = m_monitor->added.connect([this](const std::string &id) {
        addDevice(id);
    });
    m_removed = m_monitor->removed.connect([this](const std::string &id) {
        removeDevice(id);
    });
    for (const MtpMonitor::Device &device : m_monitor->devices())
        addDevice(device.id);

    m_object.addMethod("Mounts", [this] {
        return mounts();
    });
    m_object.addMethod("Mount", "sa{sv}", "sa{sv}", [this](dbus::MethodCall &call) {
        mount(call);
    }, {{"location", "options"}, {"status", "details"}});
    m_object.addMethod("Unmount", "s", "", [this](dbus::MethodCall &call) {
        unmount(call);
    }, {{"id"}, {}});
    m_object.addSignal("MountsChanged", "");
}

MountService::~MountService()
{
    *m_alive = false;
    detachAll();
}

bool MountService::publish()
{
    return m_object.publish();
}

void MountService::mountsChanged()
{
    m_object.emitSignal("MountsChanged");
}

void MountService::detachAll()
{
    for (const auto &entry : m_entries) {
        if (entry->mount)
            entry->mount->stop(true, nullptr);
    }
}

MountService::Entry *MountService::find(const std::string &id)
{
    for (const auto &entry : m_entries) {
        if (entry->id == id)
            return entry.get();
    }
    return nullptr;
}

dbus::VariantMap MountService::describe(const Entry &entry) const
{
    return {
        {"id", dbus::Variant(entry.id)},
        {"kind", dbus::Variant(entry.kind)},
        {"name", dbus::Variant(entry.name)},
        {"icon", dbus::Variant(entry.icon)},
        {"url", dbus::Variant(entry.id)},
        {"path", dbus::Variant(entry.mount ? entry.mount->mountPoint() : std::string())},
        {"network", dbus::Variant(entry.network)},
        {"device", dbus::Variant(entry.device)},
        {"ejectable", dbus::Variant(entry.device || entry.ejectable)},
    };
}

std::vector<dbus::VariantMap> MountService::mounts() const
{
    std::vector<dbus::VariantMap> result;
    for (const auto &entry : m_entries)
        result.push_back(describe(*entry));
    return result;
}

std::string MountService::mountPointFor(const std::string &name) const
{
    std::string base = strings::replaceAll(name, "/", "_");
    if (base.empty() || base.starts_with('.'))
        base.insert(base.begin(), '_');

    std::string candidate = base;
    for (int suffix = 2; exists(m_root + '/' + candidate); ++suffix)
        candidate = base + " (" + std::to_string(suffix) + ')';

    return m_root + '/' + candidate;
}

void MountService::mount(dbus::MethodCall &call)
{
    std::string location;
    dbus::VariantMap options;
    if (!call.read(location, options)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    const auto fail = [&call](const std::string &message) {
        call.reply(std::string("failed"), dbus::VariantMap {{"message", dbus::Variant(message)}});
    };

    Entry *device = find(location);
    const std::string imagePath = device ? std::string() : diskImagePath(location);
    const std::string imageId = imagePath.empty() ? std::string() : url::fromLocalFile(imagePath);
    RemoteUrl url;
    if (device)
        url = device->url;
    else if (imagePath.empty())
        url = RemoteUrl::parse(location);

    Credentials credentials;
    credentials.user = optionString(options, "user");
    credentials.domain = optionString(options, "domain");
    credentials.password = optionString(options, "password");
    credentials.anonymous = optionBool(options, "anonymous");
    credentials.acceptedHostKey = optionString(options, "hostKey");
    const bool remember = optionBool(options, "remember");

    // A server already mounted for this account opens at once.
    const RemoteUrl key = device ? RemoteUrl() : serverKey(url, url.user);
    for (const auto &entry : m_entries) {
        if (!entry->mount)
            continue;

        const RemoteUrl mounted = RemoteUrl::parse(entry->id);
        const bool same = !imageId.empty() ? entry->id == imageId
                : device                   ? entry.get() == device
                                           : !entry->device && mounted.scheme == key.scheme && mounted.host == key.host
                        && mounted.port == key.port && (key.user.empty() || mounted.user == key.user);
        if (!same)
            continue;

        const std::string path = url.path.empty() || url.path == "/" ? entry->mount->backend()->initialPath()
                                                                      : url.path;
        call.reply(std::string("mounted"),
                   dbus::VariantMap {{"path", dbus::Variant(joined(entry->mount->mountPoint(), path))}});
        return;
    }

    std::unique_ptr<Backend> backend;
    if (!imagePath.empty()) {
        backend = std::make_unique<DiskImageBackend>(imagePath);
    } else if (device) {
        const MtpMonitor::Device found = m_monitor->device(device->id);
        if (found.bus)
            backend = std::make_unique<MtpBackend>(found.bus, found.number);
    } else if (url.isValid() && !url.host.empty()) {
        backend = createBackend(url);
    }

    if (!backend) {
        fail(format(tr("“{location}” is not a location that can be connected to."), {{"location", location}}));
        return;
    }

    const std::string pendingKey = !imageId.empty() ? imageId : device ? device->id : serverId(url, credentials.user);
    if (m_pending.contains(pendingKey)) {
        fail(format(tr("Already connecting to “{location}”."), {{"location", location}}));
        return;
    }
    m_pending.insert(pendingKey);

    const std::string deviceId = device ? device->id : std::string();
    LoopHandle loop = EventLoop::current()->handle();
    auto shared = std::make_shared<std::unique_ptr<Backend>>(std::move(backend));
    // The call stays on this thread: sd-bus messages are not to be released on another.
    const int callId = m_nextCall++;
    m_calls.emplace(callId, call);

    std::thread([this, alive = m_alive, loop, callId, url, deviceId, imageId, imagePath, pendingKey, credentials,
                 remember, shared]() mutable {
        Credentials attempt = credentials;
        bool fromKeyring = false;
        if (attempt.password.empty() && !attempt.anonymous && deviceId.empty() && imageId.empty())
            fromKeyring = Keyring::find(url, &attempt);

        ConnectResult result = (*shared)->connect(attempt);

        // A saved password that no longer works is asked for again.
        if (result.status == ConnectResult::NeedPassword && fromKeyring)
            result.message.clear();

        if (result.status == ConnectResult::Connected && remember && !attempt.anonymous)
            Keyring::store(url, attempt);

        loop.post([this, alive, callId, url, deviceId, imageId, imagePath, pendingKey, result, attempt, credentials,
                   fromKeyring, shared]() mutable {
            if (!*alive)
                return;
            m_pending.erase(pendingKey);
            auto node = m_calls.extract(callId);
            dbus::MethodCall &call = node.mapped();

            std::string status;
            dbus::VariantMap details;

            switch (result.status) {
            case ConnectResult::NeedPassword:
                status = "password";
                details["user"] = dbus::Variant(!attempt.user.empty() ? attempt.user : url.user);
                details["domain"] = dbus::Variant(attempt.domain);
                details["needsDomain"] = dbus::Variant(result.needsDomain);
                details["anonymousAllowed"] = dbus::Variant(result.anonymousAllowed);
                // Typed, or kept from before: either way it was refused.
                details["retry"] = dbus::Variant(!credentials.password.empty() || fromKeyring);
                break;
            case ConnectResult::UnknownHostKey:
                status = "hostkey";
                details["fingerprint"] = dbus::Variant(result.fingerprint);
                details["changed"] = dbus::Variant(result.hostKeyChanged);
                break;
            case ConnectResult::Failed:
                status = "failed";
                details["message"] = dbus::Variant(result.message);
                break;
            case ConnectResult::Connected: {
                std::unique_ptr<Backend> backend = std::move(*shared);
                Entry *entry = deviceId.empty() ? nullptr : find(deviceId);
                std::unique_ptr<Entry> created;
                if (!entry && !imageId.empty()) {
                    created = std::make_unique<Entry>();
                    entry = created.get();
                    entry->id = imageId;
                    entry->kind = "image";
                    entry->name = static_cast<DiskImageBackend *>(backend.get())->volumeName();
                    if (entry->name.empty())
                        entry->name = imagePath.substr(imagePath.rfind('/') + 1);
                    entry->icon = "drive-removable-media";
                    entry->network = false;
                    entry->ejectable = true;
                } else if (!entry) {
                    created = std::make_unique<Entry>();
                    entry = created.get();
                    const std::string user = !attempt.user.empty() ? attempt.user : url.user;
                    entry->id = serverId(url, user);
                    entry->url = RemoteUrl::parse(entry->id);
                    entry->kind = entry->url.scheme;
                    entry->name = serverName(url, user);
                    entry->icon = "network-server";
                }

                // A device is named as it calls itself once it answers.
                if (auto *mtp = dynamic_cast<MtpBackend *>(backend.get())) {
                    const std::string name = mtp->friendlyName();
                    if (!name.empty() && name != entry->name) {
                        entry->name = name;
                        mountsChanged();
                    }
                }

                const std::string initial = url.path.empty() || url.path == "/" ? backend->initialPath() : url.path;
                const LoopHandle loop = EventLoop::current()->handle();
                auto mount = std::make_unique<FuseMount>(
                        std::move(backend), mountPointFor(entry->name), entry->id,
                        [this, alive, loop](FuseMount *mount) {
                            loop.post([this, alive, mount] {
                                if (*alive)
                                    finished(mount);
                            });
                        });

                std::string message;
                if (!mount->start(&message)) {
                    status = "failed";
                    details["message"] = dbus::Variant(message);
                    break;
                }

                status = "mounted";
                details["path"] = dbus::Variant(joined(mount->mountPoint(), initial));
                entry->mount = std::move(mount);
                if (created)
                    m_entries.push_back(std::move(created));
                mountsChanged();
                break;
            }
            }

            call.reply(status, details);
        });
    }).detach();
}

void MountService::unmount(dbus::MethodCall &call)
{
    std::string id;
    if (!call.read(id)) {
        call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
        return;
    }

    Entry *entry = find(id);
    if (!entry || !entry->mount) {
        call.reply();
        return;
    }

    std::string message;
    if (!entry->mount->stop(false, &message)) {
        call.replyError("org.lingmo.Mounts1.Error.Busy", message);
        return;
    }
    call.reply();
}

void MountService::finished(FuseMount *mount)
{
    for (auto it = m_retiring.begin(); it != m_retiring.end(); ++it) {
        if (it->get() == mount) {
            m_retiring.erase(it);
            return;
        }
    }

    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        Entry *entry = it->get();
        if (entry->mount.get() != mount)
            continue;

        entry->mount.reset();
        if (!entry->device)
            m_entries.erase(it);
        mountsChanged();
        return;
    }
}

void MountService::addDevice(const std::string &id)
{
    if (find(id))
        return;

    const MtpMonitor::Device device = m_monitor->device(id);
    auto entry = std::make_unique<Entry>();
    entry->id = id;
    entry->kind = "mtp";
    entry->name = device.name;
    entry->icon = "phone";
    entry->url = RemoteUrl::parse(id);
    entry->network = false;
    entry->device = true;
    m_entries.push_back(std::move(entry));
    mountsChanged();
}

void MountService::removeDevice(const std::string &id)
{
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
        Entry *entry = it->get();
        if (entry->id != id)
            continue;

        // Its session ends once nothing has files open there; the entry goes now.
        if (entry->mount) {
            entry->mount->stop(true, nullptr);
            m_retiring.push_back(std::move(entry->mount));
        }
        m_entries.erase(it);
        mountsChanged();
        return;
    }
}
