/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DRKONQIBACKENDS_H
#define DRKONQIBACKENDS_H

#include <QObject>

class CrashedApplication;
class DebuggerManager;

class AbstractDrKonqiBackend : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~AbstractDrKonqiBackend();
    virtual bool init();
    virtual void prepareForDebugger();

    inline CrashedApplication *crashedApplication() const
    {
        return m_crashedApplication;
    }

    inline DebuggerManager *debuggerManager() const
    {
        return m_debuggerManager;
    }

    static QString metadataPath();

Q_SIGNALS:
    void preparedForDebugger();
    void failedToPrepare();

protected:
    virtual CrashedApplication *constructCrashedApplication() = 0;
    virtual DebuggerManager *constructDebuggerManager() = 0;

private:
    CrashedApplication *m_crashedApplication = nullptr;
    DebuggerManager *m_debuggerManager = nullptr;
};

class KCrashBackend : public AbstractDrKonqiBackend
{
    Q_OBJECT
public:
    using AbstractDrKonqiBackend::AbstractDrKonqiBackend;
    ~KCrashBackend() override;

    bool init() override;

protected:
    CrashedApplication *constructCrashedApplication() override;
    DebuggerManager *constructDebuggerManager() override;

private Q_SLOTS:
    void stopAttachedProcess();
    void continueAttachedProcess();
    void onDebuggerStarting();
    void onDebuggerFinished();

private:
    static void emergencySaveFunction(int signal);
    static qint64 s_pid; // for use by the emergencySaveFunction

    enum State { ProcessRunning, ProcessStopped, DebuggerRunning };
    State m_state = ProcessRunning;
};

#endif // DRKONQIBACKENDS_H
