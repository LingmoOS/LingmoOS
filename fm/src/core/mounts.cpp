// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/mounts.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/strings.h"

#include "lingmo/files/url.h"

namespace fm {

using base::dbus::VariantMap;
using Lingmo::i18n::tr;

// Connecting asks the server and maybe the keyring; a slow server answers late.
static constexpr std::chrono::seconds kMountTimeout(120);

MountsClient& MountsClient::instance()
{
    static MountsClient client;
    return client;
}

MountsClient::MountsClient()
    : m_proxy(base::dbus::Connection::session(), "org.lingmo.Mounts1", "/org/lingmo/Mounts1",
              "org.lingmo.Mounts1")
{
    if (!m_proxy.isValid())
        return;
    m_changed = m_proxy.onSignal("MountsChanged", [this](const base::dbus::Message&) { refresh(); });
    refresh();
}

void MountsClient::call(const std::string& method, std::function<void(const base::dbus::Reply&)> done,
                        std::chrono::microseconds timeout, const std::vector<base::dbus::Variant>& arguments)
{
    base::dbus::Connection* bus = m_proxy.connection();
    if (!bus) {
        done(base::dbus::Reply({}, {"org.freedesktop.DBus.Error.Disconnected", tr("No session bus")}));
        return;
    }
    base::dbus::Message message = bus->newMethodCall(m_proxy.service(), m_proxy.path(), m_proxy.interface(), method);
    for (const base::dbus::Variant& argument : arguments)
        message.appendAs(argument);
    const int id = m_nextCall++;
    m_calls[id] = bus->callAsync(
            message,
            [this, id, done = std::move(done)](const base::dbus::Reply& reply) {
                done(reply);
                base::EventLoop::current()->post([this, id] { m_calls.erase(id); });
            },
            timeout);
}

void MountsClient::refresh()
{
    call("Mounts", [this](const base::dbus::Reply& reply) {
        const auto entries = reply.value<std::vector<VariantMap>>();
        if (!entries)
            return;
        std::vector<Mount> mounts;
        for (const VariantMap& entry : *entries) {
            const auto text = [&entry](const char* key) {
                const auto found = entry.find(key);
                return found != entry.end() ? found->second.value<std::string>() : std::string();
            };
            const auto flag = [&entry](const char* key) {
                const auto found = entry.find(key);
                return found != entry.end() && found->second.value<bool>();
            };
            mounts.push_back({text("id"), text("name"), text("icon"), text("path"), flag("network"), flag("device"),
                              flag("ejectable")});
        }
        m_mounts = std::move(mounts);
        mountsChanged.emit();
    }, {}, {});
}

MountsClient::Mount MountsClient::mount(const std::string& id) const
{
    for (const Mount& mount : m_mounts) {
        if (mount.id == id)
            return mount;
    }
    return {};
}

MountsClient::Mount MountsClient::mountAt(const std::string& path) const
{
    for (const Mount& mount : m_mounts) {
        if (!mount.path.empty() && (path == mount.path || path.starts_with(mount.path + '/')))
            return mount;
    }
    return {};
}

void MountsClient::requestMount(const std::string& location, const VariantMap& options, Reply reply)
{
    call("Mount", [reply = std::move(reply)](const base::dbus::Reply& answer) {
        std::string status;
        VariantMap details;
        if (answer.isError() || !answer.read(status, details)) {
            reply("failed", {{"message", base::dbus::Variant(answer.error().message)}});
            return;
        }
        reply(status, details);
    }, kMountTimeout, {base::dbus::Variant(location), base::dbus::Variant(options)});
}

void MountsClient::requestUnmount(const std::string& id, std::function<void(const std::string&)> reply)
{
    call("Unmount", [reply = std::move(reply)](const base::dbus::Reply& answer) {
        if (reply)
            reply(answer.isError() ? answer.error().message : std::string());
    }, {}, {base::dbus::Variant(id)});
}

std::string MountsClient::normalizedAddress(const std::string& address)
{
    std::string text(base::strings::trimmed(address));
    if (text.empty())
        return {};
    // \\server\share, as Windows writes it.
    if (text.starts_with("\\\\")) {
        text = "smb://" + base::strings::replaceAll(text.substr(2), "\\", "/");
    } else if (text.find("://") == std::string::npos) {
        text = "smb://" + text;
    }
    files::Url url(text);
    if (url.scheme() == "ssh")
        url.setScheme("sftp");
    if (!url.isValid() || url.host().empty() || (url.scheme() != "smb" && url.scheme() != "sftp"))
        return {};
    return url.strippedTrailingSlash().toString();
}

} // namespace fm
