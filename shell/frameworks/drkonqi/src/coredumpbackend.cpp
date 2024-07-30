/*
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "coredumpbackend.h"

#include <chrono>

#include <KCrash>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusReply>
#include <QDebug>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QScopeGuard>

#include <unistd.h>

// TODO: refactor the entire stack such that logs extraction happens in the processor and gets passed along the chain
#include <systemd/sd-journal.h>

#include <coredump.h>

#include "bugzillaintegration/reportinterface.h"
#include "crashedapplication.h"
#include "debugger.h"
#include "debuggermanager.h"
#include "drkonqi.h"
#include "drkonqi_debug.h"
#include "linuxprocmapsparser.h"
#include <coredumpexcavator.h>

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

namespace
{
// Special backend type for core-based debugging. A polkit helper excavates the core file for us and we then
// invoke gdb on it. Side stepping coredumpctl.
constexpr auto CORE_BACKEND_TYPE = "coredump-core"_L1;

[[nodiscard]] QString errnoError(const QString &msg, int err)
{
    return msg + u": (%1) "_s.arg(QString::number(err)) + QString::fromLocal8Bit(strerror(err));
}

QList<EntriesHash> collectLogs(const QByteArray &cursor, sd_journal *context, const QStringList &matches)
{
    Q_ASSERT(context);

    // - Reset all matches
    // - seek to the cursor of the crash
    // - install all matches to filter only log output from the crashed app
    // - grab last N messages

    sd_journal_flush_matches(context);
    if (auto err = sd_journal_seek_cursor(context, cursor.constData()); err != 0) {
        qCWarning(DRKONQI_LOG) << errnoError(u"Failed to seek to cursor"_s, -err);
        return {};
    }

    // make the cursor the current entry so we can read its time
    if (auto err = sd_journal_next(context); err < 0) {
        qCWarning(DRKONQI_LOG) << errnoError(u"Failed to read cursor"_s, -err);
        return {};
    }

    uint64_t core_time = 0;
    sd_id128_t boot_id;
    if (auto err = sd_journal_get_monotonic_usec(context, &core_time, &boot_id); err != 0) {
        qCWarning(DRKONQI_LOG) << errnoError(u"Failed to get core time"_s, -err);
        return {};
    }

    for (const auto &match : matches) {
        if (sd_journal_add_match(context, qUtf8Printable(match), 0) != 0) {
            qCWarning(DRKONQI_LOG) << "Failed to install custom match:" << match;
            return {};
        }
    }

    QList<EntriesHash> blobs;
    constexpr auto maxBacklog = 30;
    for (int i = 0; i < maxBacklog; i++) {
        if (auto err = sd_journal_previous(context); err <= 0) { // end of data
            if (err < 0) { // error
                qCWarning(DRKONQI_LOG) << errnoError(u"Failed to seek previous entry"_s, -err);
            }
            break;
        }

        uint64_t time = 0;
        if (auto err = sd_journal_get_monotonic_usec(context, &time, &boot_id); err != 0) {
            qCWarning(DRKONQI_LOG) << errnoError(u"Failed to get entry time"_s, -err);
            break; // don't know when this entry is from, assume it's old
        }

        std::chrono::microseconds timeDelta(core_time - time);
        if (timeDelta > 8s) {
            break;
        }

        EntriesHash blob;
        const void *data = nullptr;
        size_t length = 0;
        SD_JOURNAL_FOREACH_DATA(context, data, length)
        {
            // size_t is uint, QBA uses int, make sure we don't overflow the int!
            int dataSize = static_cast<int>(length);
            Q_ASSERT(dataSize >= 0);
            Q_ASSERT(static_cast<quint64>(dataSize) == length);

            QByteArray entry(static_cast<const char *>(data), dataSize);
            const auto offset = entry.indexOf('=');
            if (offset < 0) {
                qCWarning(DRKONQI_LOG) << "this entry looks funny it has no separating = character" << entry;
                continue;
            }

            const QByteArray key = entry.left(offset);
            const QByteArray value = entry.mid(offset + 1);

            blob.emplace(key, value);
        }
        blobs.push_back(blob);
    }

    return blobs;
}
} // namespace

bool CoredumpBackend::init()
{
    Q_ASSERT(!metadataPath().isEmpty());
    Q_ASSERT_X(QFile::exists(metadataPath()), static_cast<const char *>(Q_FUNC_INFO), qUtf8Printable(metadataPath()));
    qCDebug(DRKONQI_LOG) << "loading metadata" << metadataPath();

    QFile file(metadataPath());
    if (!file.open(QFile::ReadOnly)) {
        return false;
    }
    auto document = QJsonDocument::fromJson(file.readAll());
    const auto journal = document[u"journal"_s].toObject().toVariantHash();
    for (auto [key, value] : journal.asKeyValueRange()) {
        m_journalEntry.insert(key.toUtf8(), value.toByteArray());
    }
    // conceivably the file contains no Journal group for unknown reasons
    Q_ASSERT_X(!m_journalEntry.isEmpty(), static_cast<const char *>(Q_FUNC_INFO), qUtf8Printable(metadataPath()));

    AbstractDrKonqiBackend::init(); // calls constructCrashedApplication -> we need to have our members set before calling it

    if (crashedApplication()->pid() <= 0) {
        qCWarning(DRKONQI_LOG) << "Invalid pid specified or it wasn't found in journald.";
        return false;
    }

    connect(ReportInterface::self(), &ReportInterface::crashEventSent, this, [] {
        QFile file(metadataPath());
        if (!file.open(QFile::ReadWrite)) {
            qCWarning(DRKONQI_LOG) << "Failed to open for writing" << metadataPath();
            return;
        }
        auto object = QJsonDocument::fromJson(file.readAll()).object();
        constexpr auto DRKONQI_KEY = "drkonqi"_L1;
        auto drkonqiObject = object[DRKONQI_KEY].toObject();
        drkonqiObject[u"sentryEventId"_s] = ReportInterface::self()->sentryEventId();
        object[DRKONQI_KEY] = drkonqiObject;
        file.reset();
        file.write(QJsonDocument(object).toJson());
    });

    return true;
}

CrashedApplication *CoredumpBackend::constructCrashedApplication()
{
    Q_ASSERT(!m_journalEntry.isEmpty());

    bool ok = false;
    // Journald timestamps are always micro seconds, coredumpd takes care to make sure this also the case for its timestamp.
    const std::chrono::microseconds timestamp(m_journalEntry["COREDUMP_TIMESTAMP"].toLong(&ok));
    Q_ASSERT(ok);
    const auto datetime = QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count());
    Q_ASSERT(ok);
    const QFileInfo executable(QString::fromUtf8(m_journalEntry["COREDUMP_EXE"]));
    const int signal = m_journalEntry["COREDUMP_SIGNAL"].toInt(&ok);
    Q_ASSERT(ok);
    const bool hasDeletedFiles = LinuxProc::hasMapsDeletedFiles(executable.filePath(), m_journalEntry["COREDUMP_PROC_MAPS"], LinuxProc::Check::Stat);

    Q_ASSERT_X(m_journalEntry["COREDUMP_PID"].toInt() == DrKonqi::pid(),
               static_cast<const char *>(Q_FUNC_INFO),
               qPrintable(QStringLiteral("journal: %1, drkonqi: %2").arg(QString::fromUtf8(m_journalEntry["COREDUMP_PID"]), QString::number(DrKonqi::pid()))));

    auto expectedJournal = owning_ptr_call<sd_journal>(sd_journal_open, SD_JOURNAL_LOCAL_ONLY);
    Q_ASSERT(expectedJournal.ret == 0);
    Q_ASSERT(expectedJournal.value);

    m_crashedApplication = std::make_unique<CrashedApplication>(DrKonqi::pid(),
                                                                DrKonqi::thread(),
                                                                signal,
                                                                executable,
                                                                DrKonqi::appVersion(),
                                                                BugReportAddress(DrKonqi::bugAddress()),
                                                                DrKonqi::programName(),
                                                                DrKonqi::productName(),
                                                                datetime,
                                                                DrKonqi::isRestarted(),
                                                                hasDeletedFiles,
                                                                resolveApplicationNotResponding());

    m_crashedApplication->m_logs = collectLogs(m_journalEntry[Coredump::keyCursor()],
                                               expectedJournal.value.get(),
                                               {
                                                   u"_PID=%1"_s.arg(QString::fromUtf8(m_journalEntry["COREDUMP_PID"])),
                                                   u"_UID=%1"_s.arg(QString::fromUtf8(m_journalEntry["COREDUMP_UID"])),
                                                   u"_COMM=%1"_s.arg(QString::fromUtf8(m_journalEntry["COREDUMP_COMM"])),
                                                   u"_EXE=%1"_s.arg(QString::fromUtf8(m_journalEntry["COREDUMP_EXE"])),
                                                   u"_BOOT_ID=%1"_s.arg(QString::fromUtf8(m_journalEntry["_BOOT_ID"])),
                                               });

    qCDebug(DRKONQI_LOG) << "Executable is:" << executable.absoluteFilePath();
    qCDebug(DRKONQI_LOG) << "Executable exists:" << executable.exists();

    return m_crashedApplication.get();
}

DebuggerManager *CoredumpBackend::constructDebuggerManager()
{
    const QList<Debugger> internalDebuggers = Debugger::availableInternalDebuggers(CORE_BACKEND_TYPE);
    const QList<Debugger> externalDebuggers = Debugger::availableExternalDebuggers(CORE_BACKEND_TYPE);

    const Debugger preferredDebugger(Debugger::findDebugger(internalDebuggers, QStringLiteral("gdb")));
    qCDebug(DRKONQI_LOG) << "Using debugger:" << preferredDebugger.codeName();

    m_debuggerManager = new DebuggerManager(preferredDebugger, externalDebuggers, this);
    return m_debuggerManager;
}

void CoredumpBackend::prepareForDebugger()
{
    if (m_excavator) {
        m_excavator->excavateFrom(QString::fromUtf8(m_journalEntry["COREDUMP_FILENAME"]));
        return;
    }

    m_excavator = std::make_unique<AutomaticCoredumpExcavator>();
    connect(m_excavator.get(), &AutomaticCoredumpExcavator::failed, this, [this] {
        Q_EMIT failedToPrepare();
    });
    connect(m_excavator.get(), &AutomaticCoredumpExcavator::excavated, this, [this](const QString &corePath) {
        m_crashedApplication->m_coreFile = corePath;
        Q_EMIT preparedForDebugger();
    });
    m_excavator->excavateFrom(QString::fromUtf8(m_journalEntry["COREDUMP_FILENAME"]));
}

std::optional<QByteArray> CoredumpBackend::bootId() const
{
    auto ret = m_journalEntry.value("_BOOT_ID"_ba);
    if (ret.isEmpty()) {
        return {};
    }
    return ret;
}

std::optional<std::chrono::microseconds> CoredumpBackend::coreTimeSinceEpoch() const
{
    bool ok = false;
    auto ret = m_journalEntry["COREDUMP_TIMESTAMP"].toLong(&ok);
    if (!ok) {
        return {};
    }
    return std::chrono::microseconds(ret);
}

bool CoredumpBackend::resolveApplicationNotResponding() const
{
    // Note that false may mean no ANR or no conclusive ANR. I toyed with the idea of making it return an optional but
    // in truth it makes no difference. When we don't know that it is an ANR we must handle it like a regular crash anyway.

    if (DrKonqi::pid() <= 0) {
        return false; // invalid pid
    }
    if (m_journalEntry["COREDUMP_SIGNAL_NAME"] != "SIGABRT"_ba) {
        return false; // not a suitable signal
    }

    const auto bootId = m_journalEntry.value("_BOOT_ID"_ba);
    if (bootId.isEmpty()) {
        qCWarning(DRKONQI_LOG) << "Ignoring potential ANR because we found no bootid in the journal";
        return false;
    }

    bool ok = false;
    auto coreTimeSinceEpochInt = m_journalEntry["COREDUMP_TIMESTAMP"].toLong(&ok);
    if (!ok) {
        qCWarning(DRKONQI_LOG) << "Ignoring potential ANR because we found no core time in the journal";
        return false;
    }
    const std::chrono::microseconds coreTimeSinceEpoch(coreTimeSinceEpochInt);

    const auto exe = QFileInfo(QString::fromUtf8(m_journalEntry["COREDUMP_EXE"])).baseName();
    if (exe.isEmpty()) {
        qCWarning(DRKONQI_LOG) << "Ignoring potential ANR because we found no exe in the journal";
        return false;
    }

    return applicationNotRespondingFileExists(exe, bootId, coreTimeSinceEpoch);
}
bool CoredumpBackend::applicationNotRespondingFileExists(const QString &exe,
                                                         const QByteArray &bootId,
                                                         const std::chrono::microseconds &coreTimeSinceEpoch) const
{
    static const QString anrPath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/drkonqi/application-not-responding/"_L1;
    QDirIterator it(anrPath, QDir::Filter::Files);
    while (it.hasNext()) {
        it.next();
        const auto name = it.fileName();
        const auto parts = QStringView(name).split('.'_L1);
        constexpr auto partsCount = 5;
        if (parts.size() != partsCount) {
            qCWarning(DRKONQI_LOG) << "unexpected part count when parsing ANR file" << name << parts;
            continue;
        }
        const auto &partExe = parts[0];
        const auto &partBootId = parts[1];
        const auto &partPid = parts[2];
        bool partTimeValid = false;
        const auto &partTime = std::chrono::microseconds(parts[3].toLongLong(&partTimeValid));
        // rest is .json
        if (!partTimeValid) {
            continue;
        }
        if (partExe != exe) {
            continue;
        }
        if (partBootId != QString::fromUtf8(bootId)) {
            continue;
        }
        if (partPid.toInt() != DrKonqi::pid()) {
            continue;
        }
        if (partTime - coreTimeSinceEpoch > 60s) {
            qCDebug(DRKONQI_LOG) << "Ignoring potential ANR the time delta is too great" << name << partTime << coreTimeSinceEpoch;
            continue;
        }
        return true;
    }
    return false;
}

#include "moc_coredumpbackend.cpp"
