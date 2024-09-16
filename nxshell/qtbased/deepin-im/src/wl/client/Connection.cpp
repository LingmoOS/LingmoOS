// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Connection.h"

using namespace wl::client;

Connection::Connection(const std::string &name)
    : ConnectionBase()
    , display_(wl_display_connect(name.empty() ? nullptr : name.c_str()))
{
    if (!display_) {
        const char * errorMsg = "empty wayland display\r\n";
        fprintf(stderr, "Failed to connect to Wayland server: %s", errorMsg);
        return;
    }

    init();
}

Connection::~Connection() { }

void Connection::init()
{
    ConnectionBase::init();

    while (wl_display_prepare_read(display()) < 0) {
        wl_display_dispatch_pending(display());
    }
    wl_display_flush(display());
}

int Connection::getFd()
{
    return wl_display_get_fd(display());
}

bool Connection::dispatch()
{
    if (display() == nullptr) {
        return false;
    }

    if (wl_display_read_events(display()) < 0) {
        return false;
    }

    while (wl_display_prepare_read(display()) != 0) {
        if (wl_display_dispatch_pending(display()) < 0) {
            return false;
        }
    }

    flush();
    return true;
}
