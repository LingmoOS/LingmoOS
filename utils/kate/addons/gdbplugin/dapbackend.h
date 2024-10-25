/*
    SPDX-FileCopyrightText: 2022 Héctor Mesa Jiménez <wmj.py@gmx.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QObject>

#include "backendinterface.h"
#include "configview.h"
#include "dap/client.h"
#include "dap/entities.h"
#include "dap/settings.h"
#include <memory>
#include <optional>

class DapBackend : public BackendInterface
{
    Q_OBJECT
public:
    DapBackend(QObject *parent);
    ~DapBackend() override = default;

    void runDebugger(const DAPTargetConf &conf);
    bool debuggerRunning() const override;
    bool debuggerBusy() const override;
    bool hasBreakpoint(QUrl const &url, int line) const override;

    bool supportsMovePC() const override;
    bool supportsRunToCursor() const override;
    bool canSetBreakpoints() const override;
    bool canMove() const override;
    bool canContinue() const override;

    void toggleBreakpoint(QUrl const &url, int line) override;
    void movePC(QUrl const &url, int line) override;
    void runToCursor(QUrl const &url, int line) override;

    void issueCommand(QString const &cmd) override;

    QString targetName() const override;
    void setFileSearchPaths(const QStringList &paths) override;

public Q_SLOTS:
    void slotInterrupt() override;
    void slotStepInto() override;
    void slotStepOver() override;
    void slotStepOut() override;
    void slotContinue() override;
    void slotKill() override;
    void slotReRun() override;
    QString slotPrintVariable(const QString &variable) override;

    void slotQueryLocals(bool display) override;
    void changeStackFrame(int index) override;
    void changeThread(int index) override;
    void changeScope(int scopeId) override;

private:
    enum State { None, Initializing, Running, Stopped, Terminated, Disconnected, PostMortem };
    enum Task { Idle, Busy };

    void unsetClient();

    void onError(const QString &message);
    void onTerminated();
    void onStopped(const dap::StoppedEvent &info);
    void onThreads(const QList<dap::Thread> &threads);
    void onStackTrace(const int, const dap::StackTraceInfo &info);
    void onProgramEnded(int exitCode);
    void onRunning();
    void onContinuedEvent(const dap::ContinuedEvent &info);
    void onInitialized();
    void onErrorResponse(const QString &summary, const std::optional<dap::Message> &message);
    void onOutputProduced(const dap::Output &output);
    void onDebuggingProcess(const dap::ProcessInfo &info);
    void onThreadEvent(const dap::ThreadEvent &info);
    void onModuleEvent(const dap::ModuleEvent &info);
    void onScopes(const int frameId, const QList<dap::Scope> &scopes);
    void onVariables(const int variablesReference, const QList<dap::Variable> &variables);
    void onModules(const dap::ModulesInfo &modules);
    void onSourceBreakpoints(const QString &path, int reference, const std::optional<QList<dap::Breakpoint>> &breakpoints);
    void onBreakpointEvent(const dap::BreakpointEvent &info);
    void onExpressionEvaluated(const QString &expression, const std::optional<dap::EvaluateInfo> &info);
    void onGotoTargets(const dap::Source &, const int, const QList<dap::GotoTarget> &targets);
    void onCapabilitiesReceived(const dap::Capabilities &capabilities);
    void onServerDisconnected();
    void onServerFinished();

    bool tryTerminate();
    bool tryDisconnect();
    void cmdShutdown();
    void cmdHelp(const QString &cmd);
    void cmdEval(const QString &cmd);
    void cmdJump(const QString &cmd);
    void cmdRunToCursor(const QString &cmd);
    void cmdListModules(const QString &cmd);
    void cmdListBreakpoints(const QString &cmd);
    void cmdBreakpointOn(const QString &cmd);
    void cmdBreakpointOff(const QString &cmd);
    void cmdPause(const QString &cmd);
    void cmdContinue(const QString &cmd);
    void cmdNext(const QString &cmd);
    void cmdStepIn(const QString &cmd);
    void cmdStepOut(const QString &cmd);
    void cmdWhereami(const QString &cmd);

    void resetState(State state = State::Running);
    void setState(State state);
    void setTaskState(Task state);

    bool isConnectedState() const;
    bool isAttachedState() const;
    bool isRunningState() const;

    void pushRequest();
    void popRequest();

    QString resolveOrWarn(const QString &filename);
    std::optional<QString> resolveFilename(const QString &filename, bool fallback = true) const;
    dap::settings::ClientSettings &target2dap(const DAPTargetConf &target);
    std::optional<int> findBreakpoint(const QString &path, int line) const;
    std::optional<int> findBreakpointIntent(const QString &path, int line) const;

    void insertBreakpoint(const QString &path, int line);
    // return false if nothing found
    bool removeBreakpoint(const QString &path, int line);
    void informBreakpointAdded(const QString &path, const dap::Breakpoint &bpoint);
    void informBreakpointRemoved(const QString &path, int line);
    void clearBreakpoints();
    void informStackFrame();

    QString m_targetName;

    void start();

    dap::Client *m_client = nullptr;
    std::optional<dap::settings::ClientSettings> m_settings;
    State m_state;
    Task m_task;

    QString m_file;
    QString m_workDir;
    std::optional<int> m_currentThread;
    std::optional<int> m_watchedThread;
    std::optional<int> m_currentFrame;
    std::optional<int> m_currentScope;
    bool m_restart = false;
    bool m_queryLocals = false;

    enum KillMode { Polite, Force };

    struct {
        std::optional<State> target = std::nullopt;
        std::optional<KillMode> userAction = std::nullopt;
    } m_shutdown;

    void shutdownUntil(std::optional<State> state = std::nullopt);
    bool continueShutdown() const;

    struct Cursor {
        int line;
        QString path;
    };
    std::optional<Cursor> m_runToCursor;

    int m_requests;

    QStringList m_commandQueue;

    std::map<QString, QList<std::optional<dap::Breakpoint>>> m_breakpoints;
    std::map<QString, QList<dap::SourceBreakpoint>> m_wantedBreakpoints;
    QList<dap::StackFrame> m_frames;
};
