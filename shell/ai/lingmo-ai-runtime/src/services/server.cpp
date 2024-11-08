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

#include "services/server.h"
#include <iostream>
#include<sys/types.h>
#include<sys/stat.h>
#include "services/threadpool/async.h"

const std::string Server::filePath_ = "/tmp/org.lingmo.aisdk";
const std::string Server::unixPath_ = "unix:path=" + filePath_;

Server &Server::getInstance() {
    static Server instance{};

    return instance;
}

Server::Server() { init(); }

Server::~Server() { destroy(); }

bool Server::available() const {
    return cpr::GlobalThreadPool::GetInstance()->IsStarted() &&
           server_ != nullptr;
}

void Server::exec() {
    loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}

void Server::quit() {
    if (loop_ == nullptr) {
        return;
    }

    g_main_loop_quit(loop_);
}

void Server::init() {
    startThreadPool();

    startServer();
}

void Server::destroy() {
    stopServer();

    cleanupThreadPool();
}

gboolean onNewConnection(GDBusServer * /* server */,
                                 GDBusConnection *connection,
                                 gpointer userData) {
    std::cout << "on new connection" << std::endl;
    auto *server = static_cast<Server *>(userData);

    server->serviceManagerMap_.insert(
        {connection,
         std::make_unique<ServiceManager>(*connection)});

    g_object_ref(connection);

    server->closedHandlerId_ = g_signal_connect(
        connection, "closed", G_CALLBACK(server->onConnectionClosed), server);

    return true;
}

void Server::startServer() {
    auto *guid = g_dbus_generate_guid();
    GError *error = nullptr;

    // 确保该文件不存在，否则，无法使用该路径启动服务器
    unlink(filePath_.c_str());

    server_ = g_dbus_server_new_sync(unixPath_.c_str(),        /* address */
                                     G_DBUS_SERVER_FLAGS_NONE, /* flags */
                                     guid,                     /* guid */
                                     nullptr, /* GDBusAuthObserver */
                                     nullptr, /* GCancellable */
                                     &error);
    chmod(filePath_.c_str(), 0777);

    if (server_ == nullptr) {
        g_printerr("Error creating server at address %s: %s\n",
                   unixPath_.c_str(), error->message);
        g_error_free(error);

        return;
    }

    g_dbus_server_start(server_);

    g_free(guid);

    g_signal_connect(server_, "new-connection",
                     G_CALLBACK(onNewConnection), this);
}

void Server::stopServer() {
    if (server_ == nullptr) {
        return;
    }

    g_dbus_server_stop(server_);
    g_object_unref(server_);
}

void Server::startThreadPool() { cpr::async::startup(); }

void Server::cleanupThreadPool() { cpr::async::cleanup(); }

void Server::onConnectionClosed(GDBusConnection *connection,
                                gboolean /* remotePeerVanished */,
                                GError * /* error */, gpointer userData) {
    std::cout << "Connection closed." << std::endl;
    auto *server = static_cast<Server *>(userData);

    if (server->serviceManagerMap_.find(connection) ==
        server->serviceManagerMap_.end()) {
        return;
    }

    if (server->closedHandlerId_ > 0) {
        g_signal_handler_disconnect(connection, server->closedHandlerId_);
        server->closedHandlerId_ = 0;
    }

    server->serviceManagerMap_.erase(connection);

    g_object_unref(connection);
}
