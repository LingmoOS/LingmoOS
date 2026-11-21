// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/dbus.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fm {

class ChooserWindow;
class FileChooser;

// org.lingmo.FileChooser: xdg-desktop-portal-lingmo's file dialogs,
// served by the session daemon. Open takes the request as a JSON object and
// replies with the result once the dialog is answered or closed.
class ChooserService
{
public:
    explicit ChooserService(base::dbus::Connection* bus);
    ~ChooserService();

    bool hasDialogs() const { return !m_dialogs.empty(); }

private:
    struct Dialog;

    void open(base::dbus::MethodCall& call);
    void close(const std::string& caller, const std::string& id);
    void finished(Dialog* dialog, const std::string& result);
    void watch(const std::string& caller);

    base::dbus::Connection* m_bus;
    base::dbus::Object m_object;
    std::vector<std::unique_ptr<Dialog>> m_dialogs;
    std::map<std::string, base::dbus::NameWatch> m_callers;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
