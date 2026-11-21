// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QStringList>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/session_language.h"
#include "base/timer.h"

#include "processmanager.h"

namespace Lingmo {

class SessionApplication
{
public:
    SessionApplication(int argc, char **argv, Lingmo::Base::EventLoop &loop);
    ~SessionApplication();

    int exec() { return m_loop.run(); }

    void logout()             { m_processManager->logout(); }
    void reboot();
    void powerOff();
    bool suspend();
    void startDesktopProcess() { m_processManager->startDesktopProcess(); }

private:
    void endSessionOnTerminationSignals();
    void exportSession();
    void initEnvironments();
    void initLanguage();
    bool syncDBusEnvironment();
    void importSystemdEnvironment();
    void createConfigDirectory();
    int  runSync(const QString &program, const QStringList &args);

private:
    Lingmo::Base::EventLoop &m_loop;
    std::unique_ptr<SessionProcessManager> m_processManager;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_sessionObject;
    std::unique_ptr<Lingmo::Base::SessionLanguage> m_language;
    Lingmo::Base::Timer m_startTimer;
    int m_terminationWatch = 0;
};

} // namespace Lingmo
