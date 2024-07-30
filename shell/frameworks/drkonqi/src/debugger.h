/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <QString>

#include <KSharedConfig>

class Debugger
{
public:
    static QList<Debugger> availableInternalDebuggers(const QString &backend);
    static QList<Debugger> availableExternalDebuggers(const QString &backend);

    /** Returns true if this Debugger instance is valid, or false otherwise.
     * Debugger instances are valid only if they have been constructed from
     * availableInternalDebuggers() or availableExternalDebuggers(). If they
     * have been constructed directly using the Debugger constructor, they are invalid.
     */
    bool isValid() const;

    /** Returns true if this debugger is installed. This is determined by
     * looking for the executable that tryExec() returns. If it is in $PATH,
     * this method returns true.
     */
    bool isInstalled() const;

    /** Returns the translatable name of the debugger (eg. "GDB") */
    QString displayName() const;

    /** Returns the code name of the debugger (eg. "gdb"). */
    QString codeName() const;

    /** Returns the executable name that drkonqi should check if it exists
     * to determine whether the debugger is installed
     */
    QString tryExec() const;

    /** Returns a list with the drkonqi backends that this debugger supports */
    QStringList supportedBackends() const;

    /** Sets the backend to be used. This function must be called before using
     * command(), backtraceBatchCommands() or runInTerminal().
     */
    void setUsedBackend(const QString &backendName);

    /** Returns the command that should be run to use the debugger */
    QString command() const;

    /// Supports dynamic symbol resolution
    bool supportsCommandWithSymbolResolution() const;

    /** Returns the command that should be run to use the debugger with symbol resolution enabled */
    QString commandWithSymbolResolution() const;

    /** Returns the commands that should be given to the debugger when
     * run in batch mode in order to generate a backtrace
     */
    QString backtraceBatchCommands() const;

    /** Returns the commands that should be given to the debugger before
     * getting the backtrace
     */
    QString preambleCommands() const;

    /** If this is an external debugger, it returns whether it should be run in a terminal or not */
    bool runInTerminal() const;

    /** Returns the value of the arbitrary configuration parameter @param key, or an empty QString if @param key isn't defined */
    QString backendValueOfParameter(const QString &key) const;

    enum ExpandStringUsage {
        ExpansionUsagePlainText,
        ExpansionUsageShell,
    };

    static void
    expandString(QString &str, ExpandStringUsage usage = ExpansionUsagePlainText, const QString &tempFile = QString(), const QString &preambleFile = QString());

    static Debugger findDebugger(const QList<Debugger> &debuggers, const QString &defaultDebuggerCodeName);

private:
    // Similar to expandString but specifically for "staticish" expansion of commands with paths resolved at runtime.
    // Conceivably this could be changed to apply on (almost) every config read really.
    QString expandCommand(const QString &command) const;
    static QList<Debugger> availableDebuggers(const QString &path, const QString &backend);
    KSharedConfig::Ptr m_config;
    QString m_backend;
};

#endif
