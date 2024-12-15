// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_CLIENT_CONNECTIONFD_H
#define WL_CLIENT_CONNECTIONFD_H

#include "ConnectionBase.h"

#include <wayland-client-protocol.h>

#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace wl {
namespace client {

class ConnectionRaw : public ConnectionBase
{
public:
    ConnectionRaw(struct wl_display *display);
    virtual ~ConnectionRaw();

    struct wl_display *display() const override { return display_; }

private:
    struct wl_display *display_;
};

} // namespace client
} // namespace wl

#endif // !WL_CLIENT_CONNECTIONFD_H

