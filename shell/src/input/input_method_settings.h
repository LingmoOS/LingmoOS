// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/settings.h"
#include "base/signal.h"

namespace Lingmo::Input {

class DBusExport;

// An input method a desktop file offers, as KDE marks them with
// X-KDE-Wayland-VirtualKeyboard=true.
struct InputMethodProvider {
    std::string id;
    std::string path;
    std::vector<std::string> command;
};

std::vector<std::string> parseDesktopExec(const std::string &exec);
std::vector<InputMethodProvider>
findInputMethodProviders(const std::vector<std::string> &applicationDirs);

class InputMethodSettings
{
public:
    // Empty arguments select the user's settings file and the XDG application directories.
    explicit InputMethodSettings(const std::string &settingsFile = {},
                                 const std::vector<std::string> &applicationDirs = {});
    ~InputMethodSettings();
    bool registerService();

    // The selected desktop file id, or empty when none is selected or installed.
    std::string inputMethod() const;

    std::vector<std::string> command() const;
    void setRunning(bool running);
    // Launches `id` for this session only; the saved selection is unchanged.
    void overrideInputMethod(const std::string &id);

    std::vector<std::string> availableInputMethods() const;
    bool setInputMethod(const std::string &id);

    base::Signal<> inputMethodChanged;
    base::Signal<> runningChanged;
    base::Signal<> commandChanged;

private:
    std::optional<InputMethodProvider> provider(const std::string &id) const;
    void notify(const base::Signal<> &signal, const char *member);

    base::Settings m_settings;
    std::vector<std::string> m_applicationDirs;
    std::string m_selected;
    bool m_running = false;
    std::unique_ptr<DBusExport> m_object;
};

} // namespace Lingmo::Input
