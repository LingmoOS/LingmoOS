/*****************************************************************
 * drkonqi - The KDE Crash Handler
 *
 * SPDX-FileCopyrightText: 2000-2003 Hans Petter Bieker <bieker@kde.org>
 * SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *****************************************************************/
#include "backtracegenerator.h"

#include "config-drkonqi.h"
#include "drkonqi.h"
#include "drkonqi_debug.h"

#include <QNetworkInformation>
#include <QTemporaryDir>

#include <KProcess>
#include <KShell>

#include "parser/backtraceparser.h"
#include "settings.h"

bool isMeteredNetwork()
{
    if (!QNetworkInformation::loadBackendByFeatures(QNetworkInformation::Feature::Metered)) {
        qCDebug(DRKONQI_LOG) << "Failed to load QNetworkInformation backend";
        return false;
    }

    if (!QNetworkInformation::instance()) {
        qCDebug(DRKONQI_LOG) << "No QNetworkInformation instance";
        return false;
    }

    const auto metered = QNetworkInformation::instance()->isMetered();
    qCDebug(DRKONQI_LOG) << "Is metered:" << metered;
    return metered;
}

BacktraceGenerator::BacktraceGenerator(const Debugger &debugger, QObject *parent)
    : QObject(parent)
    , m_debugger(debugger)
    , m_supportsSymbolResolution(WITH_GDB12 && m_debugger.supportsCommandWithSymbolResolution())
    , m_symbolResolution(m_debugger.supportsCommandWithSymbolResolution() && Settings::self()->downloadSymbols() && !isMeteredNetwork())
{
    m_parser = BacktraceParser::newParser(m_debugger.codeName(), this);
    m_parser->connectToGenerator(this);

#ifdef BACKTRACE_PARSER_DEBUG
    m_debugParser = BacktraceParser::newParser(QString(), this); // uses the null parser
    m_debugParser->connectToGenerator(this);
#endif
}

BacktraceGenerator::~BacktraceGenerator()
{
    if (m_proc && m_proc->state() == QProcess::Running) {
        qCWarning(DRKONQI_LOG) << "Killing running debugger instance";
        m_proc->disconnect(this);
        m_proc->terminate();
        if (!m_proc->waitForFinished(10000)) {
            m_proc->kill();
            // lldb can become "stuck" on OS X; just mark m_proc as to be deleted later rather
            // than waiting a potentially very long time for it to heed the kill() request.
            m_proc->deleteLater();
        } else {
            delete m_proc;
        }
        delete m_temp;
    }
}

void BacktraceGenerator::start()
{
    // they should always be null before entering this function.
    Q_ASSERT(!m_proc);
    Q_ASSERT(!m_temp);

    m_parsedBacktrace.clear();

    if (!m_debugger.isValid() || !m_debugger.isInstalled()) {
        qCWarning(DRKONQI_LOG) << "Debugger valid" << m_debugger.isValid() << "installed" << m_debugger.isInstalled();
        m_state = FailedToStart;
        Q_EMIT stateChanged();
        Q_EMIT failedToStart();
        return;
    }

    m_state = Loading;
    Q_EMIT stateChanged();
    Q_EMIT preparing();
    // DebuggerManager calls setBackendPrepared when it is ready for us to actually start.
}

void BacktraceGenerator::slotReadInput()
{
    if (!m_proc) {
        // this can happen with lldb after we detected that it detached from the debuggee.
        return;
    }

    // we do not know if the output array ends in the middle of an utf-8 sequence
    m_output += m_proc->readAllStandardOutput();

    int pos;
    while ((pos = m_output.indexOf('\n')) != -1) {
        QString line = QString::fromLocal8Bit(m_output.constData(), pos + 1);
        m_output.remove(0, pos + 1);

        Q_EMIT newLine(line);
        line = line.simplified();
        if (line.startsWith(QLatin1String("Process ")) && line.endsWith(QLatin1String(" detached"))) {
            // lldb is acting on a detach command (in lldbrc)
            // Anything following this line doesn't interest us, and lldb has been known
            // to turn into a zombie instead of exiting, thereby blocking us.
            // Tell the process to quit if it's still running, and pretend it did.
            if (m_proc && m_proc->state() == QProcess::Running) {
                m_proc->terminate();
                if (!m_proc->waitForFinished(500)) {
                    m_proc->kill();
                }
                if (m_proc) {
                    slotProcessExited(0, QProcess::NormalExit);
                }
            }
            return;
        }
    }
}

void BacktraceGenerator::resetProcess()
{
    if (m_proc) {
        m_proc->deleteLater();
        m_proc = nullptr;
    }
    if (m_temp) {
        m_temp->deleteLater();
        m_temp = nullptr;
    }
}

void BacktraceGenerator::slotProcessExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    // the process is useless now
    resetProcess();

    // mark the end of the backtrace for the parser
    Q_EMIT newLine(QString());

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        m_state = Failed;
        Q_EMIT stateChanged();
        Q_EMIT someError();
        return;
    }

    // no translation, string appears in the report
    QString tmp(QStringLiteral("Application: %progname (%execname), signal: %signame\n"));
    Debugger::expandString(tmp);

    m_parsedBacktrace = tmp + m_parser->informationLines() + m_parser->parsedBacktrace();
    m_sentryPayload = [this]() -> QByteArray {
        const QString sentryPayloadFile = m_tempDirectory->path() + QLatin1String("/sentry_payload.json");
        QFile file(sentryPayloadFile);
        if (!file.open(QFile::ReadOnly)) {
            qCWarning(DRKONQI_LOG) << "Could not open sentry payload file" << sentryPayloadFile;
            return {};
        }
        return file.readAll();
    }();
    m_state = Loaded;
    Q_EMIT stateChanged();

#ifdef BACKTRACE_PARSER_DEBUG
    // append the raw unparsed backtrace
    m_parsedBacktrace += "\n------------ Unparsed Backtrace ------------\n";
    m_parsedBacktrace += m_debugParser->parsedBacktrace(); // it's not really parsed, it's from the null parser.
#endif

    Q_EMIT done();
}

void BacktraceGenerator::slotOnErrorOccurred(QProcess::ProcessError error)
{
    qCWarning(DRKONQI_LOG) << "Debugger process had an error" << error << m_proc->program() << m_proc->arguments() << m_proc->environment();

    // make very sure the process is getting discarded, otherwise retry operations won't work
    resetProcess();

    switch (error) {
    case QProcess::FailedToStart:
        m_state = FailedToStart;
        Q_EMIT stateChanged();
        Q_EMIT failedToStart();
        break;
    default:
        m_state = Failed;
        Q_EMIT stateChanged();
        Q_EMIT someError();
        break;
    }
}

void BacktraceGenerator::setBackendPrepared()
{
    // they should always be null before entering this function.
    Q_ASSERT(!m_proc);
    Q_ASSERT(!m_temp);

    Q_ASSERT(m_state == Loading);

    Q_EMIT starting();

    m_proc = new KProcess;
    m_proc->setEnv(QStringLiteral("LC_ALL"), QStringLiteral("C.UTF-8")); // force C locale

    // Temporary directory for the preamble.py to write data into, we can then conveniently pick it up from there.
    // Only useful for data that is not meant to appear in the trace (e.g. sentry payloads).
    if (!m_tempDirectory) {
        m_tempDirectory = std::make_unique<QTemporaryDir>();
    }
    if (!m_tempDirectory->isValid()) {
        qCWarning(DRKONQI_LOG) << "Failed to create temporary directory for generator!";
    } else {
        m_proc->setEnv(QStringLiteral("DRKONQI_TMP_DIR"), m_tempDirectory->path());
        m_proc->setEnv(QStringLiteral("DRKONQI_VERSION"), QStringLiteral(PROJECT_VERSION));
        m_proc->setEnv(QStringLiteral("DRKONQI_APP_VERSION"), DrKonqi::appVersion());
        m_proc->setEnv(QStringLiteral("DRKONQI_SIGNAL"), QString::number(DrKonqi::signal()));
    }

    m_temp = new QTemporaryFile;
    m_temp->open();
    m_temp->write(m_debugger.backtraceBatchCommands().toLatin1());
    m_temp->write("\n", 1);
    m_temp->flush();

    auto preamble = new QTemporaryFile(m_proc);
    preamble->open();
    preamble->write(m_debugger.preambleCommands().toUtf8());
    preamble->write("\n", 1);
    preamble->flush();

    // start the debugger
    QString str = m_symbolResolution ? m_debugger.commandWithSymbolResolution() : m_debugger.command();
    Debugger::expandString(str, Debugger::ExpansionUsageShell, m_temp->fileName(), preamble->fileName());

    *m_proc << KShell::splitArgs(str);
    m_proc->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    m_proc->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Text);
    // check if the debugger should take its input from a file we'll generate,
    // and take the appropriate steps if so
    QString stdinFile = m_debugger.backendValueOfParameter(QStringLiteral("ExecInputFile"));
    Debugger::expandString(stdinFile, Debugger::ExpansionUsageShell, m_temp->fileName(), preamble->fileName());
    if (!stdinFile.isEmpty() && QFile::exists(stdinFile)) {
        m_proc->setStandardInputFile(stdinFile);
    }
    connect(m_proc, &KProcess::readyReadStandardOutput, this, &BacktraceGenerator::slotReadInput);
    connect(m_proc, static_cast<void (KProcess::*)(int, QProcess::ExitStatus)>(&KProcess::finished), this, &BacktraceGenerator::slotProcessExited);
    connect(m_proc, &KProcess::errorOccurred, this, &BacktraceGenerator::slotOnErrorOccurred);

    qCDebug(DRKONQI_LOG) << "Starting debugger" << m_proc->program() << m_proc->arguments();
    m_proc->start();
}

bool BacktraceGenerator::debuggerIsGDB() const
{
    return m_debugger.codeName() == QLatin1String("gdb");
}

QString BacktraceGenerator::debuggerName() const
{
    return m_debugger.displayName();
}

QByteArray BacktraceGenerator::sentryPayload() const
{
    return m_sentryPayload;
};

void BacktraceGenerator::setBackendFailed()
{
    // Shouldn't have been set yet
    Q_ASSERT(!m_proc);
    Q_ASSERT(!m_temp);
    m_proc = nullptr;
    m_temp = nullptr;

    m_state = FailedToStart;
    Q_EMIT stateChanged();
    Q_EMIT failedToStart();
}

bool BacktraceGenerator::hasAnyFailure()
{
    return m_state == State::Failed || m_state == State::FailedToStart;
}

#include "moc_backtracegenerator.cpp"
