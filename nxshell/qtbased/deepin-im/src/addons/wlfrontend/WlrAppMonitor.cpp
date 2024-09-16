#include "WlrAppMonitor.h"

#include "ForeignToplevelManagementV1.h"
#include "TreelandForeignToplevelManagementV1.h"
#include "wl/client/ConnectionBase.h"

#include <QDebug>

using namespace org::deepin::dim;

WlrAppMonitor::WlrAppMonitor(const std::shared_ptr<wl::client::ConnectionBase> &conn)
    : conn_(conn)
{
    toplevelManager_ = conn->getGlobal<ForeignToplevelManagerV1>();
    if (toplevelManager_) {
        toplevelManager_->setRefreshCallback([this]() {
            refresh();
        });
    }

    treelandToplevelManager_ = conn->getGlobal<TreelandForeignToplevelManagerV1>();
    if (treelandToplevelManager_) {
        treelandToplevelManager_->setRefreshCallback([this]() {
            treelandRefresh();
        });
    }
}

WlrAppMonitor::~WlrAppMonitor() = default;

void WlrAppMonitor::refresh()
{
    std::unordered_map<std::string, std::string> state;
    std::string focus;

    const auto &list = toplevelManager_->list();
    for (auto &i : list) {
        if (i->appId().empty()) {
            continue;
        }

        state.emplace(i->key(), i->appId());
        if (i->activated()) {
            focus = i->key();
        }
    }

    emit appUpdated(state, focus);
}

void WlrAppMonitor::treelandRefresh()
{
    std::unordered_map<std::string, std::string> state;
    std::string focus;

    const auto &treelandToplevelHandlelist = treelandToplevelManager_->list();
    for (auto &i : treelandToplevelHandlelist) {
        if (i->appId().empty()) {
            continue;
        }

        state.emplace(i->key(), i->appId());
        if (i->activated()) {
            focus = i->key();
        }
    }

    emit appUpdated(state, focus);
}
