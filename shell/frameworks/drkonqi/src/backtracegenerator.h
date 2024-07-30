/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * SPDX-FileCopyrightText: 2000-2003 Hans Petter Bieker <bieker@kde.org>
 * SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *****************************************************************/

#ifndef BACKTRACEGENERATOR_H
#define BACKTRACEGENERATOR_H

#include <memory>

#include <QProcess>
#include <QTemporaryFile>

#include "debugger.h"

class KProcess;
class BacktraceParser;
class QTemporaryDir;

class BacktraceGenerator : public QObject
{
    Q_OBJECT

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool hasAnyFailure READ hasAnyFailure NOTIFY stateChanged) // derives from state
    Q_PROPERTY(bool supportsSymbolResolution MEMBER m_supportsSymbolResolution CONSTANT)
    Q_PROPERTY(bool symbolResolution MEMBER m_symbolResolution NOTIFY symbolResolutionChanged)
public:
    enum State {
        NotLoaded,
        Loading,
        Loaded,
        Failed,
        FailedToStart,
    };
    Q_ENUM(State)

    BacktraceGenerator(const Debugger &debugger, QObject *parent);
    ~BacktraceGenerator() override;

    State state() const
    {
        return m_state;
    }

    Q_INVOKABLE BacktraceParser *parser() const
    {
        return m_parser;
    }

    Q_INVOKABLE QString backtrace() const
    {
        return m_parsedBacktrace;
    }

    // Called by manager when it is ready for us.
    void setBackendPrepared();
    // ... or not
    void setBackendFailed();

    Q_INVOKABLE bool debuggerIsGDB() const;
    Q_INVOKABLE QString debuggerName() const;
    QByteArray sentryPayload() const;

public Q_SLOTS:
    void start();
    bool hasAnyFailure();

Q_SIGNALS:
    void starting();
    void newLine(const QString &str); // emitted for every line
    void someError();
    void failedToStart();
    void done();
    void preparing();
    void stateChanged();
    void symbolResolutionChanged();

private Q_SLOTS:
    void slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotReadInput();
    void slotOnErrorOccurred(QProcess::ProcessError error);

private:
    void resetProcess();
    const Debugger m_debugger;
    KProcess *m_proc = nullptr;
    QTemporaryFile *m_temp = nullptr;
    QByteArray m_output;
    State m_state = NotLoaded;
    BacktraceParser *m_parser = nullptr;
    QString m_parsedBacktrace;
    std::unique_ptr<QTemporaryDir> m_tempDirectory;
    const bool m_supportsSymbolResolution = false;
    bool m_symbolResolution;
    QByteArray m_sentryPayload;

#ifdef BACKTRACE_PARSER_DEBUG
    BacktraceParser *m_debugParser = nullptr;
#endif
};

#endif
