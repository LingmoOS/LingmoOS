// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/signal.h"
#include "pages/input/compositorservice.h"

namespace settings {

// Client of the compositor's org.lingmo.InputMethod1 service.
class InputMethod
{
public:
    struct Method
    {
        // The desktop entry's id; empty for none.
        std::string id;
        std::string name;
        // A theme icon name; empty when the entry gives none.
        std::string icon;
    };

    InputMethod();

    bool isAvailable() const { return m_service.isAvailable(); }
    const std::vector<Method>& methods() const { return m_methods; }
    std::string current() const;
    bool isRunning() const;

    // An empty id turns the input method off.
    void select(const std::string& id);

    Lingmo::base::Signal<>& changed() { return m_service.changed; }

private:
    void refreshMethods();

    CompositorService m_service;
    Lingmo::base::Subscription m_serviceChanged;
    bool m_wasAvailable = false;
    std::vector<Method> m_methods;
};

} // namespace settings
