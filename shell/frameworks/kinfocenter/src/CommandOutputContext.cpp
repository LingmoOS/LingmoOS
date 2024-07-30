/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
*/

#include "CommandOutputContext.h"

#include <utility>

#include <QDebug>
#include <QProcess>
#include <QStandardPaths>

#include <KLocalizedString>
#include <KOSRelease>

CommandOutputContext::CommandOutputContext(const QStringList &findExecutables, const QString &executable, const QStringList &arguments, QObject *parent)
    : QObject(parent)
    , m_executableName(executable)
    , m_executablePath(QStandardPaths::findExecutable(m_executableName))
    , m_arguments(arguments)
    , m_bugReportUrl(KOSRelease().bugReportUrl())
{
    // Various utilities are installed in sbin, but work without elevated privileges
    if (m_executablePath.isEmpty()) {
        m_executablePath =
            QStandardPaths::findExecutable(m_executableName, {QStringLiteral("/usr/local/sbin"), QStringLiteral("/usr/sbin"), QStringLiteral("/sbin")});
    }

    m_foundExecutablePaths[executable] = m_executablePath;
    for (const QString &findExecutable : findExecutables) {
        m_foundExecutablePaths[findExecutable] = QStandardPaths::findExecutable(findExecutable);
    }

    metaObject()->invokeMethod(this, &CommandOutputContext::load);
}

CommandOutputContext::CommandOutputContext(const QString &executable, const QStringList &arguments, QObject *parent)
    : CommandOutputContext({/* executable is by default always searched for */}, executable, arguments, parent)
{
}

QString CommandOutputContext::executableName() const
{
    return m_executableName;
}

QStringList CommandOutputContext::arguments() const
{
    return m_arguments;
}

QString CommandOutputContext::filter() const
{
    return m_filter;
}

void CommandOutputContext::setFilter(const QString &filter)
{
    m_filter = filter;
    if (m_filter.isEmpty()) {
        m_text = m_originalLines.join('\n');
    } else {
        m_text.clear();
        for (const QString &line : std::as_const(m_originalLines)) {
            if (line.contains(filter, Qt::CaseInsensitive)) {
                m_text += line + '\n';
            }
        }
    }
    Q_EMIT textChanged();
    Q_EMIT filterChanged();
}

void CommandOutputContext::reset()
{
    m_ready = false;
    m_error.clear();
    m_explanation.clear();
    m_text.clear();
    m_filter.clear();
    Q_EMIT readyChanged();
    Q_EMIT errorChanged();
    Q_EMIT explanationChanged();
    Q_EMIT textChanged();
    Q_EMIT filterChanged();

    // Not exposed as properties
    m_originalLines.clear();
}

void CommandOutputContext::load()
{
    reset();

    for (auto it = m_foundExecutablePaths.cbegin(); it != m_foundExecutablePaths.cend(); ++it) {
        if (it.value().isEmpty()) {
            setError(xi18nc("@info", "The <command>%1</command> tool is required to display this page, but could not be found", it.key()),
                     xi18nc("@info",
                            "You can search for it and install it using your package manager.<nl/>"
                            "Then please report this packaging issue to your distribution."));
            return;
        }
    }

    auto proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, &QProcess::finished, this, [this, proc](int /* exitCode */, QProcess::ExitStatus exitStatus) {
        proc->deleteLater();

        switch (exitStatus) {
        case QProcess::CrashExit:
            return setError(xi18nc("@info", "The <command>%1</command> tool crashed while generating page content", m_executableName),
                            xi18nc("@Info", "Try again later. If keeps happening, please report the crash to your distribution."));
        case QProcess::NormalExit:
            break;
        }

        m_text = QString::fromLocal8Bit(proc->readAllStandardOutput());
        if (m_trimAllowed) {
            m_text = m_text.trimmed();
        }
        m_originalLines = m_text.split('\n');
        if (!m_filter.isEmpty()) {
            // re-apply filter on new text
            setFilter(m_filter);
        }

        Q_EMIT textChanged();
        setReady();
    });
    proc->start(m_executablePath, m_arguments);
}

void CommandOutputContext::setError(const QString &message, const QString &explanation = QString())
{
    m_error = message;

    if (!explanation.isEmpty()) {
        m_explanation = explanation;
    }

    Q_EMIT errorChanged();
    Q_EMIT explanationChanged();

    setReady();
}

void CommandOutputContext::setReady()
{
    m_ready = true;
    Q_EMIT readyChanged();
}

void CommandOutputContext::setTrimAllowed(bool allow)
{
    m_trimAllowed = allow;
}

#include "moc_CommandOutputContext.cpp"
