// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <memory>

#include "base/dbus.h"

namespace Lingmo::Shell {

class GtkSettings
{
public:
    static constexpr const char *kModules = "appmenu-gtk-module";

    explicit GtkSettings(base::dbus::Connection *bus);
    ~GtkSettings();

    GtkSettings(const GtkSettings &) = delete;
    GtkSettings &operator=(const GtkSettings &) = delete;

    bool registerService();

private:
    base::dbus::Connection *m_bus;
    std::unique_ptr<base::dbus::Object> m_object;
};

} // namespace Lingmo::Shell
