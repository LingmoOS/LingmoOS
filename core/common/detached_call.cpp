// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/detached_call.h"

#include <cerrno>

#include <systemd/sd-bus.h>

namespace Lingmo::Core {

using Callback = std::function<void(const base::dbus::Reply&)>;

static base::dbus::Error errorOf(const sd_bus_error* error)
{
    return {error && error->name ? error->name : SD_BUS_ERROR_FAILED,
            error && error->message ? error->message : ""};
}

static int replied(sd_bus_message* message, void* data, sd_bus_error*)
{
    const Callback& done = *static_cast<Callback*>(data);
    if (!done)
        return 0;
    const base::dbus::Error error = sd_bus_message_is_method_error(message, nullptr)
            ? errorOf(sd_bus_message_get_error(message))
            : base::dbus::Error();
    done(base::dbus::Reply(base::dbus::Message(message), error));
    return 0;
}

static void deleteCallback(void* data)
{
    delete static_cast<Callback*>(data);
}

void callDetached(base::dbus::Connection* connection, const base::dbus::Message& call,
                  std::function<void(const base::dbus::Reply&)> done, std::chrono::microseconds timeout)
{
    auto* callback = new Callback(std::move(done));
    sd_bus_slot* slot = nullptr;
    const int r = !connection ? -ENOTCONN
            : call.isNull()   ? -EINVAL
                              : sd_bus_call_async(connection->bus(), &slot, call.get(), replied, callback,
                                                  uint64_t(timeout.count()));
    if (r < 0) {
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_error_set_errno(&error, r);
        if (*callback)
            (*callback)(base::dbus::Reply({}, errorOf(&error)));
        sd_bus_error_free(&error);
        delete callback;
        return;
    }
    sd_bus_slot_set_destroy_callback(slot, deleteCallback);
    // Floating, the slot belongs to the bus: freed after the reply, or with the
    // bus when the connection closes first.
    sd_bus_slot_set_floating(slot, 1);
    sd_bus_slot_unref(slot);
}

} // namespace Lingmo::Core
