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

#ifndef SERVICES_SERVERPOXY_H
#define SERVICES_SERVERPOXY_H

#include <string>

#include <gio/gio.h>

class ServerProxy {
public:
    static ServerProxy &getInstance();

    ~ServerProxy();

    static const std::string &getUnixPath();

    bool available() const { return connection_ != nullptr; }

    GDBusConnection *getConnection() const { return connection_; }

    void exec();

    void quit();

private:
    ServerProxy();

    void init();
    void destroy();

private:
    GDBusConnection *connection_ = nullptr;

    GMainLoop *loop_ = nullptr;
};

#endif