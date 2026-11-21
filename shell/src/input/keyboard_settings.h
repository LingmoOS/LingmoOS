// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "base/settings.h"
#include "base/signal.h"
#include "input/types.h"

namespace Lingmo::Input {

class DBusExport;

class KeyboardSettings
{
public:
    static constexpr int kMinRepeatDelayMs = 150;
    static constexpr int kMaxRepeatDelayMs = 2000;
    static constexpr int kMinRepeatIntervalMs = 20;
    static constexpr int kMaxRepeatIntervalMs = 2000;

    // Compiles and installs a layout name; false when it cannot be used.
    using LayoutApplier = std::function<bool(const std::string &)>;

    explicit KeyboardSettings(const std::string &settingsFile = {});
    ~KeyboardSettings();
    bool registerService();

    // Installs the saved layout right away. A saved layout that no longer
    // compiles falls back to the default.
    void setLayoutApplier(LayoutApplier applier);

    bool repeatEnabled() const { return m_repeatEnabled; }

    int repeatDelay() const { return m_repeatDelay; }

    int repeatInterval() const { return m_repeatInterval; }

    // "layout" or "layout-variant", e.g. "de-neo"; the xkb default when the
    // user never picked one.
    std::string layout() const;

    ShortcutConvention shortcutConvention() const { return m_shortcutConvention; }

    // "pc" or "command"; see ShortcutConvention.
    std::string shortcutConventionName() const;

    void setRepeatEnabled(bool enabled);
    void setRepeatDelay(int milliseconds);
    void setRepeatInterval(int milliseconds);
    // An empty name goes back to the default.
    bool setLayout(const std::string &name);
    // False for a name that is not a convention.
    bool setShortcutConvention(const std::string &name);

    base::Signal<> repeatEnabledChanged;
    base::Signal<> repeatDelayChanged;
    base::Signal<> repeatIntervalChanged;
    base::Signal<> layoutChanged;
    base::Signal<> shortcutConventionChanged;

private:
    // No value removes the key.
    template<typename T>
    void save(const std::string &key, const std::optional<T> &value);
    void notify(const base::Signal<> &signal, const char *member);

    base::Settings m_settings;
    LayoutApplier m_applier;
    bool m_repeatEnabled = true;
    int m_repeatDelay = 0;
    int m_repeatInterval = 0;
    std::string m_layout;
    ShortcutConvention m_shortcutConvention = ShortcutConvention::Pc;
    std::unique_ptr<DBusExport> m_object;
};

} // namespace Lingmo::Input
