// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <map>

#include <QString>
#include <QStringList>
#include <QList>

#include "base/dbus.h"
#include "base/process.h"
#include "base/signal.h"
#include "base/timer.h"

namespace Lingmo {

class XwaylandEnvironment;
class CompositorProcess;
class ShellConnection;
class ShellProcess;
class RestartPolicy;
struct Publisher;

class SessionProcessManager
{
public:
    SessionProcessManager();
    ~SessionProcessManager();

    enum class Ending {
        Logout,
        Restart,
        Shutdown,
    };

    void start();
    void logout();

    // Asks every application to quit, and runs `then` once all have.
    // One that cancels or stops responding keeps the session going,
    // and the user is told which. `then` returns false when the session
    // goes on after all, as after a refused restart.
    void endAfterApplications(Ending ending, std::function<bool()> then);

    // What logout does once the display manager has been told, for when
    // it or the system is the one ending the session.
    void endSession(bool behindLoginScreen = false);

    void startDesktopProcess();
    void startDaemonProcess();
    void loadAutoStartProcess();

private:
    void leave();
    void notifyEndingStopped(Ending ending, const QString &outcome,
                              const QString &applicationId);
    void startAfterCompositorReady();
    void publishXwaylandEnvironment(bool startDesktop);
    void startShell();
    void onShellFinished(int code, bool crashed);

    // Started once and restarted by the same bounded rule as the shell.
    void startCoreProcess(const QString &program,
                           const QStringList &arguments = {});
    void stopRestarts();
    void stopShellAndDaemons();
    void stopProcesses(std::map<QString, std::unique_ptr<Lingmo::Base::Process>> &processes);

private:
    std::unique_ptr<XwaylandEnvironment> m_xwaylandEnvironment;
    std::unique_ptr<CompositorProcess> m_compositorProcess;
    std::unique_ptr<ShellConnection> m_shellConnection;
    std::unique_ptr<ShellProcess> m_shellProcess;
    std::unique_ptr<RestartPolicy> m_shellRestart;
    Lingmo::Base::DBus::NameWatch m_servicesWatch;
    Lingmo::Base::DBus::PendingCall m_quitAllCall;
    Lingmo::Base::Subscription m_registryChanged;
    Lingmo::Base::Timer m_autostartTimer;
    Lingmo::Base::Timer m_exitTimer;
    std::vector<std::unique_ptr<Publisher>> m_publishers;
    bool m_started = false;
    bool m_compositorReady = false;
    bool m_desktopStarted = false;
    bool m_loggingOut = false;
    bool m_ending = false;
    QString m_waylandSocket;
    QString m_controlSocket;
    std::map<QString, std::unique_ptr<Lingmo::Base::Process>> m_coreProcesses;
    std::map<QString, std::unique_ptr<RestartPolicy>> m_restartPolicies;
};

} // namespace Lingmo
