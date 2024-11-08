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

#ifndef SERVICES_SERVER_H
#define SERVICES_SERVER_H

#include <map>
#include <memory>
#include <string>

#include <gio/gio.h>
#include <gio/giotypes.h>

#include "services/servicemanager.h"
#include "engine/aienginepluginmanager.h"

class Server {
public:
    static Server &getInstance();

    ~Server();

    bool available() const;

    void exec();

    void quit();

    static const std::string &getUnixPath() { return unixPath_; }

private:
    Server();

    void init();
    void destroy();

    void startServer();
    void stopServer();

    void startThreadPool();
    void cleanupThreadPool();

    friend gboolean onNewConnection(GDBusServer *server,
                                    GDBusConnection *connection,
                                    gpointer userData);

    static void onConnectionClosed(GDBusConnection *connection,
                                   gboolean remotePeerVanished, GError *Error,
                                   gpointer userData);

private:
    static const std::string filePath_;
    static const std::string unixPath_;

    ai_engine::AiEnginePluginManager aiEnginePluginManager_;

    std::map<GDBusConnection *, std::unique_ptr<ServiceManager>>
        serviceManagerMap_;

    gulong closedHandlerId_ = 0;

    GDBusServer *server_ = nullptr;
    GMainLoop *loop_ = nullptr;
};

#endif
