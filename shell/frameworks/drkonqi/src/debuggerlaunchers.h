/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DEBUGGERLAUNCHERS_H
#define DEBUGGERLAUNCHERS_H

#include <QDBusAbstractAdaptor>

#include "debugger.h"
#include "debuggermanager.h"

class DetachedProcessMonitor;

class AbstractDebuggerLauncher : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
public:
    explicit AbstractDebuggerLauncher(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    virtual QString name() const = 0;

public Q_SLOTS:
    virtual void start() = 0;

Q_SIGNALS:
    void starting();
    void finished();
    void invalidated();
};

class DefaultDebuggerLauncher : public AbstractDebuggerLauncher
{
    Q_OBJECT
public:
    explicit DefaultDebuggerLauncher(const Debugger &debugger, DebuggerManager *parent = nullptr);
    QString name() const override;

public Q_SLOTS:
    void start() override;

private Q_SLOTS:
    void onProcessFinished();

private:
    const Debugger m_debugger;
    DetachedProcessMonitor *m_monitor = nullptr;
};

class DBusInterfaceAdaptor;

/** This class handles the old drkonqi dbus interface used by kdevelop */
class DBusInterfaceLauncher : public AbstractDebuggerLauncher
{
    Q_OBJECT
public:
    explicit DBusInterfaceLauncher(const QString &name, qint64 pid, DBusInterfaceAdaptor *parent = nullptr);
    QString name() const override;

public Q_SLOTS:
    void start() override;

private:
    QString m_name;
    qint64 m_pid;
};

class DBusInterfaceAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.drkonqi")
public:
    explicit DBusInterfaceAdaptor(DebuggerManager *parent);

public Q_SLOTS:
    int pid();
    Q_NOREPLY void registerDebuggingApplication(const QString &name, qint64 pid = 0);
    Q_NOREPLY void debuggingFinished(const QString &name);
    Q_NOREPLY void debuggerClosed(const QString &name);

Q_SIGNALS:
    void acceptDebuggingApplication(const QString &name);

private:
    QHash<QString, DBusInterfaceLauncher *> m_launchers;
};

#endif // DEBUGGERLAUNCHERS_H
