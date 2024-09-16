// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLRAPPMONITOR_H
#define WLRAPPMONITOR_H

#include "AppMonitor.h"

#include <memory>

namespace wl::client {
class ConnectionBase;
} // namespace wl::client

namespace org::deepin::dim {

class ForeignToplevelManagerV1;
class TreelandForeignToplevelManagerV1;

class WlrAppMonitor : public AppMonitor
{
public:
    WlrAppMonitor(const std::shared_ptr<wl::client::ConnectionBase> &conn);
    ~WlrAppMonitor() override;

    void refresh();
    void treelandRefresh();

private:
    const std::shared_ptr<wl::client::ConnectionBase> conn_;
    std::shared_ptr<ForeignToplevelManagerV1> toplevelManager_;
    std::shared_ptr<TreelandForeignToplevelManagerV1> treelandToplevelManager_;
};

} // namespace org::deepin::dim

#endif // !WLRAPPMONITOR_H
