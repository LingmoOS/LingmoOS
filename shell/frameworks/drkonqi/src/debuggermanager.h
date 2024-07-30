/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DEBUGGERMANAGER_H
#define DEBUGGERMANAGER_H

#include <QObject>

class BacktraceGenerator;
class Debugger;
class AbstractDebuggerLauncher;
class AbstractDrKonqiBackend;

class DebuggerManager : public QObject
{
    Q_OBJECT
public:
    explicit DebuggerManager(const Debugger &internalDebugger, const QList<Debugger> &externalDebuggers, AbstractDrKonqiBackend *backendParent);
    ~DebuggerManager() override;

    bool debuggerIsRunning() const;
    bool showExternalDebuggers() const;
    QList<AbstractDebuggerLauncher *> availableExternalDebuggers() const;
    BacktraceGenerator *backtraceGenerator() const;
    void addDebugger(AbstractDebuggerLauncher *launcher, bool emitsignal = false);

Q_SIGNALS:
    void debuggerStarting();
    void debuggerFinished();
    void debuggerRunning(bool running);
    void externalDebuggerAdded(AbstractDebuggerLauncher *launcher);
    void externalDebuggerRemoved(AbstractDebuggerLauncher *launcher);

private Q_SLOTS:
    void onDebuggerStarting();
    void onDebuggerFinished();
    void onDebuggerInvalidated();

private:
    struct Private;
    Private *const d;
};

#endif // DEBUGGERMANAGER_H
