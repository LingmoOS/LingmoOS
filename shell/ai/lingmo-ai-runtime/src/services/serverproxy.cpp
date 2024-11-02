/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "services/serverproxy.h"

#include "services/server.h"

ServerProxy &ServerProxy::getInstance() {
    static ServerProxy instance{};

    return instance;
}

ServerProxy::ServerProxy() { init(); }

ServerProxy::~ServerProxy() { destroy(); }

const std::string &ServerProxy::getUnixPath() { return Server::getUnixPath(); }

void ServerProxy::exec() {
    loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}

void ServerProxy::quit() {
    if (loop_ == nullptr) {
        return;
    }

    g_main_loop_quit(loop_);
}

void ServerProxy::init() {
    const auto &unixPath = Server::getUnixPath();
    GError *error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
        nullptr, /* GDBusAuthObserver */
        nullptr, /* GCancellable */
        &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n",
                   unixPath.c_str(), error->message);
        g_error_free(error);

        return;
    }
}

void ServerProxy::destroy() {
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}