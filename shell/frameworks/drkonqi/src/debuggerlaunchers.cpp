/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "debuggerlaunchers.h"

#include <QCoreApplication>
#include <QDBusConnection>

#include "drkonqi_debug.h"
#include <KProcess>
#include <KShell>

#include "crashedapplication.h"
#include "detachedprocessmonitor.h"
#include "drkonqi.h"

#include "ptracer.h"

DefaultDebuggerLauncher::DefaultDebuggerLauncher(const Debugger &debugger, DebuggerManager *parent)
    : AbstractDebuggerLauncher(parent)
    , m_debugger(debugger)
{
    m_monitor = new DetachedProcessMonitor(this);
    connect(m_monitor, &DetachedProcessMonitor::processFinished, this, &DefaultDebuggerLauncher::onProcessFinished);
}

QString DefaultDebuggerLauncher::name() const
{
    return m_debugger.displayName();
}

void DefaultDebuggerLauncher::start()
{
    if (static_cast<DebuggerManager *>(parent())->debuggerIsRunning()) {
        qCWarning(DRKONQI_LOG) << "Another debugger is already running";
        return;
    }

    QString str = m_debugger.command();
    Debugger::expandString(str, Debugger::ExpansionUsageShell);

    Q_EMIT starting();
    int pid = KProcess::startDetached(KShell::splitArgs(str));
    if (pid > 0) {
        setPtracer(pid, DrKonqi::pid());
        m_monitor->startMonitoring(pid);
    } else {
        qCWarning(DRKONQI_LOG) << "Could not start debugger:" << name();
        Q_EMIT finished();
    }
}

void DefaultDebuggerLauncher::onProcessFinished()
{
    setPtracer(QCoreApplication::applicationPid(), DrKonqi::pid());
    Q_EMIT finished();
}

DBusInterfaceLauncher::DBusInterfaceLauncher(const QString &name, qint64 pid, DBusInterfaceAdaptor *parent)
    : AbstractDebuggerLauncher(parent)
    , m_name(name)
    , m_pid(pid)
{
}

QString DBusInterfaceLauncher::name() const
{
    return m_name;
}

void DBusInterfaceLauncher::start()
{
    Q_EMIT starting();

    setPtracer(m_pid, DrKonqi::pid());

    Q_EMIT static_cast<DBusInterfaceAdaptor *>(parent())->acceptDebuggingApplication(m_name);
}

DBusInterfaceAdaptor::DBusInterfaceAdaptor(DebuggerManager *parent)
    : QDBusAbstractAdaptor(parent)
{
    Q_ASSERT(parent);

    if (QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.drkonqi.PID%1").arg(pid()))) {
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/debugger"), parent);
    }
}

int DBusInterfaceAdaptor::pid()
{
    return DrKonqi::crashedApplication()->pid();
}

void DBusInterfaceAdaptor::registerDebuggingApplication(const QString &name, qint64 pid)
{
    if (!name.isEmpty() && !m_launchers.contains(name)) {
        auto launcher = new DBusInterfaceLauncher(name, pid, this);
        m_launchers.insert(name, launcher);
        static_cast<DebuggerManager *>(parent())->addDebugger(launcher, true);
    }
}

void DBusInterfaceAdaptor::debuggingFinished(const QString &name)
{
    auto it = m_launchers.find(name);
    if (it != m_launchers.end()) {
        setPtracer(QCoreApplication::applicationPid(), DrKonqi::pid());
        Q_EMIT it.value()->finished();
    }
}

void DBusInterfaceAdaptor::debuggerClosed(const QString &name)
{
    auto it = m_launchers.find(name);
    if (it != m_launchers.end()) {
        Q_EMIT it.value()->invalidated();
        m_launchers.erase(it);
    }
}

#include "moc_debuggerlaunchers.cpp"
