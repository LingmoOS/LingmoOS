// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "base/dbus.h"
#include "base/signal.h"
#include <cstdint>
#include <functional>
#include <memory>
namespace Photos {
class GlobalMenu {
public:
    explicit GlobalMenu(Lingmo::base::dbus::Connection*, std::function<void(int32_t)> activate);
    ~GlobalMenu();
    void setViewerActive(bool active);
    Lingmo::base::Signal<bool> availabilityChanged;
private:
    void registerMenu();
    Lingmo::base::dbus::Connection* m_bus;
    std::function<void(int32_t)> m_activate;
    Lingmo::base::dbus::Object m_object;
    Lingmo::base::dbus::NameWatch m_watch;
    Lingmo::base::dbus::PendingCall m_registration;
    uint32_t m_revision = 1;
    uint64_t m_generation = 0;
    bool m_available = false;
    bool m_viewerActive = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
} // namespace Photos
