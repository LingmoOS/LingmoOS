// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "base/settings.h"
#include "base/signal.h"

namespace Lingmo::Input {

class DBusExport;

// Lists the connected touchscreens by name and keeps which of them the user
// turned off. Exported as org.lingmo.Touchscreen1 for Settings.
class TouchscreenSettings
{
public:
    explicit TouchscreenSettings(const std::string &settingsFile = {});
    ~TouchscreenSettings();
    bool registerService();

    const std::vector<std::string> &devices() const { return m_devices; }

    // Names, not device nodes, so a choice survives a reboot or a replug.
    // Disabled screens stay listed after they are unplugged.
    const std::vector<std::string> &disabledDevices() const { return m_disabledDevices; }

    bool deviceEnabled(const std::string &name) const
    {
        return std::find(m_disabledDevices.begin(), m_disabledDevices.end(), name)
                == m_disabledDevices.end();
    }

    void setDevices(const std::vector<std::string> &devices);

    void setDeviceEnabled(const std::string &name, bool enabled);

    base::Signal<> devicesChanged;
    base::Signal<> disabledDevicesChanged;
    base::Signal<> settingsChanged;

private:
    void notify(const base::Signal<> &signal, const char *member);

    base::Settings m_settings;
    std::vector<std::string> m_devices;
    std::vector<std::string> m_disabledDevices;
    std::unique_ptr<DBusExport> m_object;
};

} // namespace Lingmo::Input
