// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/signal.h"
#include "pages/input/compositorservice.h"

namespace settings {

struct KeyboardLayout
{
    // "layout" or "layout-variant", as xkeyboard-config names them.
    std::string id;
    std::string name;
};

// Every layout and variant in the xkb rules file, sorted by name. Read on
// first use.
const std::vector<KeyboardLayout>& keyboardLayouts();
// Reads a rules file such as /usr/share/X11/xkb/rules/evdev.xml.
std::vector<KeyboardLayout> readKeyboardLayouts(const std::string& path);

// Client of the compositor's org.lingmo.Keyboard1 service.
class Keyboard
{
public:
    Keyboard();

    bool isAvailable() const { return m_service.isAvailable(); }

    bool repeatEnabled() const;
    void setRepeatEnabled(bool enabled);
    int repeatDelay() const;
    void setRepeatDelay(int milliseconds);
    int repeatInterval() const;
    void setRepeatInterval(int milliseconds);

    std::string layout() const;
    std::string layoutName() const;
    void selectLayout(const std::string& id);

    // "pc", or "command" for the key next to the space bar working as Command.
    std::string shortcutConvention() const;
    void setShortcutConvention(const std::string& convention);

    Lingmo::base::Signal<>& changed() { return m_service.changed; }

private:
    CompositorService m_service;
};

} // namespace settings
