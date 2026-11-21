// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell.h"
#include "shell/theme_follower.h"

#include <malloc.h>

#include <algorithm>
#include <chrono>

#include "base/dbus.h"
#include "base/settings.h"
#include "lingmo/appearance/appearance.h"
#include "core/preferences.h"
#include "decoration/decoration_theme.h"
#include "display/display_manager.h"
#include "shell/common/app_launcher.h"
#include "shell/common/application_badges.h"
#include "shell/common/applications.h"
#include "shell/common/file_thread.h"
#include "shell/common/task_manager.h"
#include "shell/common/wm_task_windows.h"
#include "shell/shell_host.h"
#include "shell/shell_input.h"
#include "shell/window_renderer.h"
#include "toolkit/toolkit_host.h"
#include "ui/application.h"
#include "ui/theme.h"
#include "wm/overview/overview_controller.h"
#include "wm/window_manager.h"

namespace Lingmo::Shell {

// FishUI's Theme: dark mode and accent from the
// Appearance service, as the compositor's own theme follows them, and the
// icon theme and the font from services.conf.
class Shell::ThemeFollower : public ShellThemeFollower {
public:
    using ShellThemeFollower::ShellThemeFollower;
};

Shell::Shell(const ShellEnvironment &environment)
    : m_environment(environment)
{
    // Before any window: it sets the toolkit up on the main loop.
    ui::Application &application = toolkit::application();
    // The compositor's toolkit has no session bus of its own: the shell's
    // follows lingmo-services for it.
    if (bus())
        application.followSessionLanguage(bus());
    // The overview belongs to the window manager, which has no toolkit.
    m_languageChanged = application.languageChanged.connect([this] {
        if (wm::OverviewController *overview = windowManager()->overview())
            overview->retranslate();
    });
    m_renderer = std::make_unique<WindowRenderer>(m_environment.compositor);
    m_input = std::make_unique<ShellInput>(this);
    m_host = std::make_unique<ShellHost>(this, m_environment.inputMethod);
    m_theme = std::make_unique<ThemeFollower>(m_environment.theme, m_environment.servicesSettings);
    m_outputsChanged = windowManager()->outputsChanged.connect([this] { updateOutputs(); });
    updateOutputs();
    m_trimTimer.setSingleShot(true);
    m_trimTimer.onTimeout = [] {
        // Trimming walks every arena; the main thread waits only for its own.
        fileThread().post([] { malloc_trim(0); });
    };
    // Startup decodes icons, parses SVGs and compiles shaders.
    releaseFreedMemorySoon();
}

Shell::~Shell()
{
    // Components first, while the windows they own can still reach the input
    // client and the renderer.
    while (!m_components.empty())
        m_components.pop_back();
    m_outputsChanged.reset();
    m_languageChanged.reset();
    m_badges.reset();
    m_launcher.reset();
    m_tasks.reset();
    m_taskWindows.reset();
    m_applications.reset();
    m_theme.reset();
    m_host.reset();
    m_input.reset();
    m_renderer.reset();
}

display::DisplayManager *Shell::displayManager() const
{
    return windowManager()->displayManager();
}

const Shell::Output *Shell::output(const std::string &name) const
{
    if (m_outputs.empty())
        return nullptr;
    const auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                                 [&](const Output &output) { return output.name == name; });
    return it != m_outputs.end() ? &*it : &m_outputs.front();
}

const Shell::Output *Shell::outputAt(const base::Point &point) const
{
    if (m_outputs.empty())
        return nullptr;
    const auto it = std::find_if(m_outputs.begin(), m_outputs.end(),
                                 [&](const Output &output) { return output.bounds.contains(point); });
    return it != m_outputs.end() ? &*it : &m_outputs.front();
}

Applications &Shell::applications()
{
    if (!m_applications)
        m_applications = std::make_unique<RegistryApplications>();
    return *m_applications;
}

TaskWindows &Shell::taskWindows()
{
    if (!m_taskWindows)
        m_taskWindows = std::make_unique<WmTaskWindows>(*windowManager());
    return *m_taskWindows;
}

TaskManager &Shell::tasks()
{
    if (!m_tasks)
        m_tasks = std::make_unique<TaskManager>(taskWindows(), applications());
    return *m_tasks;
}

ApplicationLauncher &Shell::launcher()
{
    if (!m_launcher)
        m_launcher = std::make_unique<ApplicationLauncher>(bus(), m_environment.createActivationToken);
    return *m_launcher;
}

ApplicationBadges &Shell::badges()
{
    if (!m_badges)
        m_badges = std::make_unique<ApplicationBadges>(bus());
    return *m_badges;
}

void Shell::openSettings(const std::vector<std::string> &arguments)
{
    // A running Settings takes the request from the new process, and comes
    // forward with its token.
    const std::string token = m_environment.createActivationToken ? m_environment.createActivationToken()
                                                                  : std::string();
    ApplicationLauncher::startDetached("lingmo-settings", arguments,
                                       token.empty() ? std::vector<std::string>()
                                                     : std::vector<std::string> {"XDG_ACTIVATION_TOKEN=" + token});
}

void Shell::setApplications(std::unique_ptr<Applications> applications)
{
    m_applications = std::move(applications);
}

void Shell::setTaskWindows(std::unique_ptr<TaskWindows> windows)
{
    m_taskWindows = std::move(windows);
}

base::Rect Shell::workArea(const Output &output) const
{
    return windowManager()->restingWorkAreaForOutput(output.bounds);
}

void Shell::releaseFreedMemorySoon()
{
    m_trimTimer.start(std::chrono::seconds(10));
}

void Shell::addWindow(ShellWindow *window)
{
    m_windows.push_back(window);
}

void Shell::removeWindow(ShellWindow *window)
{
    m_windows.erase(std::remove(m_windows.begin(), m_windows.end(), window), m_windows.end());
}

void Shell::updateOutputs()
{
    std::vector<Output> outputs;
    const display::DisplayManager *displays = displayManager();
    if (displays && displays->applied()) {
        for (const auto &logical : displays->applied()->logicalDisplays) {
            if (logical.bounds.isEmpty())
                continue;
            outputs.push_back(
                    {logical.waylandName, logical.bounds, logical.scale.value(), logical.primary});
        }
    }
    if (outputs.empty()) {
        for (const wm::WindowManager::OutputArea &area : windowManager()->outputAreas())
            outputs.push_back({{}, area.bounds, area.scale, outputs.empty()});
    }
    std::stable_partition(outputs.begin(), outputs.end(),
                          [](const Output &output) { return output.primary; });
    if (outputs == m_outputs)
        return;
    m_outputs = std::move(outputs);
    outputsChanged();
}

} // namespace Lingmo::Shell
