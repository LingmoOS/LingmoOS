// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"

namespace Lingmo::base::dbus {
class Connection;
}

namespace Lingmo::Compositor {
class Compositor;
}

namespace Lingmo::Decoration {
class Theme;
}

namespace Lingmo::Display {
class DisplayManager;
}

namespace Lingmo::Input {
class Seat;
}

namespace Lingmo::Wayland {
class InputMethodRelay;
}

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::Shell {

class ApplicationBadges;
class ApplicationLauncher;
class Applications;
class ShellHost;
class ShellInput;
class TaskManager;
class TaskWindows;
class ShellWindow;
class WindowRenderer;

// One part of the desktop shell: the dock, the status bar, the launcher.
// Created and owned by the Shell, destroyed before anything it uses.
class Component
{
public:
    virtual ~Component() = default;
};

// What the shell runs on, as CompositorShell owns it.
struct ShellEnvironment {
    compositor::Compositor *compositor = nullptr;
    wm::WindowManager *windowManager = nullptr;
    input::Seat *seat = nullptr;
    // The session bus on the main loop; null where there is none (tests).
    base::dbus::Connection *bus = nullptr;
    // The input method's relay for the shell's text fields; null for none.
    wayland::InputMethodRelay *inputMethod = nullptr;
    // The compositor's appearance: dark mode, accent and corners, which the
    // toolkit's theme follows. Null keeps the toolkit's light theme.
    decoration::Theme *theme = nullptr;
    // The session's services.conf, for the icon theme and the font; empty
    // reads the user's.
    std::string servicesSettings;
    // An xdg-activation token for a program the shell starts on the user's
    // input; null where there is none (tests).
    std::function<std::string()> createActivationToken;
};

// The desktop shell inside the compositor, after ash::Shell's shell parts: it
// owns the components and what they share, its internal windows' input
// client, their renderer and the toolkit's host. Created by CompositorShell outside the greeter.
class Shell
{
public:
    // A logical output as the shell lays itself out on it.
    struct Output {
        // The wl_output name; empty for the single output of a compositor
        // without a display manager.
        std::string name;
        base::Rect bounds;
        double scale = 1;
        bool primary = false;

        bool operator==(const Output &) const = default;
    };

    explicit Shell(const ShellEnvironment &environment);
    Shell(const Shell &) = delete;
    Shell &operator=(const Shell &) = delete;
    ~Shell();

    compositor::Compositor *compositor() const { return m_environment.compositor; }
    wm::WindowManager *windowManager() const { return m_environment.windowManager; }
    input::Seat *seat() const { return m_environment.seat; }
    base::dbus::Connection *bus() const { return m_environment.bus; }
    display::DisplayManager *displayManager() const;

    ShellInput *input() const { return m_input.get(); }
    // What the toolkit's windows ask of the compositor; popups, text input
    // and drags live there.
    ShellHost *host() const { return m_host.get(); }
    WindowRenderer *renderer() const { return m_renderer.get(); }

    // Every output, the primary one first.
    const std::vector<Output> &outputs() const { return m_outputs; }
    // The output named `name`; the primary one for an empty or unknown name.
    // Null when there is no output at all.
    const Output *output(const std::string &name) const;
    // The output containing `point`, else the primary one.
    const Output *outputAt(const base::Point &point) const;
    // An output came, went, or changed its bounds or scale.
    base::Signal<> outputsChanged;
    // The output's bounds less what the shell's windows and the separate
    // shell reserved, as the window manager lays windows out in it now.
    base::Rect workArea(const Output &output) const;

    // What the components share, made on first use: the installed
    // applications, the application windows, which application each belongs
    // to, application starts and the applications' counts.
    Applications &applications();
    TaskWindows &taskWindows();
    TaskManager &tasks();
    ApplicationLauncher &launcher();
    ApplicationBadges &badges();
    // Starts Settings with `arguments`, such as {"-m", "sound"}, and brings a
    // running Settings forward, minimized or hidden as it may be.
    void openSettings(const std::vector<std::string> &arguments = {});
    // Tests hand in their own before the first use.
    void setApplications(std::unique_ptr<Applications> applications);
    void setTaskWindows(std::unique_ptr<TaskWindows> windows);

    // Components are destroyed in reverse order of addition, before the shell.
    template <typename T>
    T *addComponent(std::unique_ptr<T> component)
    {
        T *result = component.get();
        m_components.push_back(std::move(component));
        return result;
    }

    // Hands what malloc keeps of freed memory back to the system a few
    // seconds later, off the main thread. Components call it after a burst
    // of transient allocations, e.g. once a large window was hidden.
    void releaseFreedMemorySoon();

    // Called by ShellWindow; not part of the public API.
    void addWindow(ShellWindow *window);
    void removeWindow(ShellWindow *window);
    const std::vector<ShellWindow *> &windows() const { return m_windows; }

private:
    class ThemeFollower;

    void updateOutputs();

    const ShellEnvironment m_environment;
    std::vector<Output> m_outputs;
    std::vector<ShellWindow *> m_windows;
    std::unique_ptr<WindowRenderer> m_renderer;
    std::unique_ptr<ShellInput> m_input;
    std::unique_ptr<ShellHost> m_host;
    std::unique_ptr<ThemeFollower> m_theme;
    std::unique_ptr<Applications> m_applications;
    std::unique_ptr<TaskWindows> m_taskWindows;
    std::unique_ptr<TaskManager> m_tasks;
    std::unique_ptr<ApplicationLauncher> m_launcher;
    std::unique_ptr<ApplicationBadges> m_badges;
    base::Subscription m_outputsChanged;
    base::Subscription m_languageChanged;
    base::Timer m_trimTimer;
    // Last: the first to go.
    std::vector<std::unique_ptr<Component>> m_components;
};

} // namespace Lingmo::Shell

