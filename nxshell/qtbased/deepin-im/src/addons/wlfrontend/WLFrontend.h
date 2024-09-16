// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLFRONTEND_H
#define WLFRONTEND_H

#include "wl/client/ConnectionBase.h"

#include <dimcore/FrontendAddon.h>

#include <QDBusObjectPath>

#include <memory>

namespace wl {
namespace client {
class ConnectionBase;
class Compositor;
class Surface;
class Seat;
} // namespace client
} // namespace wl

namespace org {
namespace deepin {
namespace dim {

class AppMonitor;
class WaylandInputContext;

class WLFrontend : public FrontendAddon
{
    Q_OBJECT

public:
    explicit WLFrontend(Dim *dim);
    ~WLFrontend() override;

    wl::client::ConnectionBase *getWl() { return wl_.get(); }

private:
    std::shared_ptr<wl::client::ConnectionBase> wl_;
    std::shared_ptr<wl::client::Compositor> compositor_;
    std::shared_ptr<wl::client::Surface> surface_;
    std::unordered_map<std::shared_ptr<wl::client::Seat>, std::unique_ptr<WaylandInputContext>>
        ims_;

    std::shared_ptr<AppMonitor> appMonitor_;

    void reloadSeats();
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !WLFRONTEND_H
