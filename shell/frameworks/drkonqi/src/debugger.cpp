/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "debugger.h"

#include <KConfig>
#include <KConfigGroup>
#include <KFileUtils>
#include <KMacroExpander>
#include <QCoreApplication>
#include <QDir>

#include "crashedapplication.h"
#include "drkonqi.h"
#include "drkonqi_debug.h"

// static
QList<Debugger> Debugger::availableInternalDebuggers(const QString &backend)
{
    return availableDebuggers(QStringLiteral("debuggers/internal"), backend);
}

// static
QList<Debugger> Debugger::availableExternalDebuggers(const QString &backend)
{
    return availableDebuggers(QStringLiteral("debuggers/external"), backend);
}

bool Debugger::isValid() const
{
    return m_config;
}

bool Debugger::isInstalled() const
{
    QString tryexec = tryExec();
    if (tryexec.isEmpty()) {
        qCDebug(DRKONQI_LOG) << "tryExec of" << codeName() << "is empty!";
        return false;
    }

    // Find for executable in PATH and in our application path
    return !QStandardPaths::findExecutable(tryexec).isEmpty() || !QStandardPaths::findExecutable(tryexec, {QCoreApplication::applicationDirPath()}).isEmpty();
}

QString Debugger::displayName() const
{
    return isValid() ? m_config->group(QStringLiteral("General")).readEntry("Name") : QString();
}

QString Debugger::codeName() const
{
    // fall back to the "TryExec" string if "CodeName" is not specified.
    // for most debuggers those strings should be the same
    return isValid() ? m_config->group(QStringLiteral("General")).readEntry("CodeName", tryExec()) : QString();
}

QString Debugger::tryExec() const
{
    return isValid() ? m_config->group(QStringLiteral("General")).readEntry("TryExec") : QString();
}

QStringList Debugger::supportedBackends() const
{
    return isValid() ? m_config->group(QStringLiteral("General")).readEntry("Backends").split(QLatin1Char('|'), Qt::SkipEmptyParts) : QStringList();
}

void Debugger::setUsedBackend(const QString &backendName)
{
    if (supportedBackends().contains(backendName)) {
        m_backend = backendName;
    }
}

QString Debugger::command() const
{
    if (!isValid() || !m_config->hasGroup(m_backend)) {
        return {};
    }
    return expandCommand(m_config->group(m_backend).readPathEntry("Exec", QString()));
}

bool Debugger::supportsCommandWithSymbolResolution() const
{
    if (!isValid() || !m_config->hasGroup(m_backend)) {
        return false;
    }
    return m_config->group(m_backend).hasKey("ExecWithSymbolResolution");
}

QString Debugger::commandWithSymbolResolution() const
{
    if (!isValid() || !m_config->hasGroup(m_backend)) {
        return {};
    }
    return expandCommand(m_config->group(m_backend).readPathEntry("ExecWithSymbolResolution", command()));
}

QString Debugger::backtraceBatchCommands() const
{
    if (!isValid() || !m_config->hasGroup(m_backend)) {
        return {};
    }
    return expandCommand(m_config->group(m_backend).readPathEntry("BatchCommands", QString()));
}

QString Debugger::preambleCommands() const
{
    if (!isValid() || !m_config->hasGroup(m_backend)) {
        return {};
    }
    return expandCommand(m_config->group(m_backend).readPathEntry("PreambleCommands", QString()));
}

QString Debugger::expandCommand(const QString &command) const
{
    static QHash<QString, QString> map = {
        {QStringLiteral("drkonqi_datadir"), QStandardPaths::locate(QStandardPaths::AppDataLocation, codeName(), QStandardPaths::LocateDirectory)},
    };
    return KMacroExpander::expandMacros(command, map);
}

bool Debugger::runInTerminal() const
{
    return (isValid() && m_config->hasGroup(m_backend)) ? m_config->group(m_backend).readEntry("Terminal", false) : false;
}

QString Debugger::backendValueOfParameter(const QString &key) const
{
    return (isValid() && m_config->hasGroup(m_backend)) ? m_config->group(m_backend).readEntry(key, QString()) : QString();
}

// static
void Debugger::expandString(QString &str, ExpandStringUsage usage, const QString &tempFile, const QString &preambleFile)
{
    const CrashedApplication *appInfo = DrKonqi::crashedApplication();
    const QHash<QString, QString> map = {
        {QLatin1String("progname"), appInfo->name()},
        {QLatin1String("execname"), appInfo->fakeExecutableBaseName()},
        {QLatin1String("execpath"), appInfo->executable().absoluteFilePath()},
        {QLatin1String("signum"), QString::number(appInfo->signalNumber())},
        {QLatin1String("signame"), appInfo->signalName()},
        {QLatin1String("pid"), QString::number(appInfo->pid())},
        {QLatin1String("tempfile"), tempFile},
        {QLatin1String("preamblefile"), preambleFile},
        {QLatin1String("thread"), QString::number(appInfo->thread())},
        {QStringLiteral("corefile"), appInfo->m_coreFile},
    };

    if (usage == ExpansionUsageShell) {
        str = KMacroExpander::expandMacrosShellQuote(str, map);
    } else {
        str = KMacroExpander::expandMacros(str, map);
    }
}

// static
QList<Debugger> Debugger::availableDebuggers(const QString &path, const QString &backend)
{
    const QStringList debuggerDirs{// Search from application path, this helps when deploying an application
                                   // as binary blob (e.g. Windows exe).
                                   QCoreApplication::applicationDirPath() + QLatin1Char('/') + path,
                                   // Search in default path
                                   QStandardPaths::locate(QStandardPaths::AppDataLocation, path, QStandardPaths::LocateDirectory)};
    const QStringList debuggerFiles = KFileUtils::findAllUniqueFiles(debuggerDirs);

    QList<Debugger> result;
    for (const auto &debuggerFile : debuggerFiles) {
        Debugger debugger;
        debugger.m_config = KSharedConfig::openConfig(debuggerFile);
        if (debugger.supportedBackends().contains(backend)) {
            debugger.setUsedBackend(backend);
            result << debugger;
        }
    }
    return result;
}

Debugger Debugger::findDebugger(const QList<Debugger> &debuggers, const QString &defaultDebuggerCodeName)
{
    Debugger firstKnownGoodDebugger;
    Debugger preferredDebugger;
    for (const Debugger &debugger : debuggers) {
        qCDebug(DRKONQI_LOG) << "Check debugger if" << debugger.displayName() << "[" << debugger.codeName() << "]"
                             << "is installed:" << debugger.isInstalled();
        if (!firstKnownGoodDebugger.isValid() && debugger.isInstalled()) {
            firstKnownGoodDebugger = debugger;
        }
        if (debugger.codeName() == defaultDebuggerCodeName) {
            preferredDebugger = debugger;
        }
        if (firstKnownGoodDebugger.isValid() && preferredDebugger.isValid()) {
            break;
        }
    }

    if (!preferredDebugger.isInstalled()) {
        if (firstKnownGoodDebugger.isValid()) {
            preferredDebugger = firstKnownGoodDebugger;
        } else {
            qCWarning(DRKONQI_LOG) << "Unable to find an internal debugger that can work with the crash backend";
        }
    }

    return preferredDebugger;
}
