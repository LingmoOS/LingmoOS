// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/dbus.h"
#include "base/signal.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace fm {

class PreviewController;
class PreviewWindow;

// org.lingmo.FilePreview: the one Quick Look window of the session, for the
// file manager's windows and the desktop alike, each a source with its own id.
class PreviewService
{
public:
    explicit PreviewService(base::dbus::Connection* bus);
    ~PreviewService();

    bool isVisible() const;
    std::function<void(bool visible)> onVisibilityChanged;

private:
    void toggle(const std::string& source, const std::string& caller, const std::string& snapshot);
    void apply(const std::string& source, const std::string& caller, const std::string& snapshot, bool activate);
    bool owns(const std::string& source, const std::string& caller) const;
    void watchOwner();

    base::dbus::Connection* m_bus;
    base::dbus::Object m_object;
    std::unique_ptr<PreviewController> m_controller;
    std::unique_ptr<PreviewWindow> m_window;
    std::vector<base::Subscription> m_subscriptions;
    base::dbus::NameWatch m_ownerWatch;
    std::string m_source;
    std::string m_owner;
};

} // namespace fm
