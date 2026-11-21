// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "requestobject.h"

#include "base/event_loop.h"
#include "base/strings.h"

using namespace Lingmo;

std::unique_ptr<base::dbus::Object> exportRequest(base::dbus::Connection *bus, const std::string &path,
                                                  std::function<void(base::dbus::MethodCall &)> close)
{
    if (!base::strings::startsWith(path, "/org/freedesktop/portal/desktop/request/"))
        return nullptr;

    auto object = std::make_unique<base::dbus::Object>(bus, path, "org.freedesktop.impl.portal.Request");
    object->addMethod("Close", "", "", [close = std::move(close)](base::dbus::MethodCall &call) {
        close(call);
        if (!call.isReplied())
            call.reply();
    });
    if (!object->publish())
        return nullptr;
    return object;
}

base::dbus::NameWatch watchRequester(base::dbus::Connection *bus, const std::string &service,
                                     std::function<void()> gone)
{
    return bus->watchName(service, [gone = std::move(gone)](const std::string &owner) {
        if (owner.empty())
            gone();
    });
}

void later(const std::shared_ptr<bool> &alive, std::function<void()> task)
{
    base::EventLoop::current()->post([alive = std::weak_ptr<bool>(alive), task = std::move(task)] {
        if (!alive.expired())
            task();
    });
}
