/*
    SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>
*/

#include "coredumpwatcher.h"

#include <cerrno>
#include <optional>
#include <utility>

#include <sys/resource.h>
#include <sys/un.h>
#include <unistd.h>

#include "coredump.h"
#include "socket.h"

using namespace Qt::StringLiterals;

static std::optional<Coredump> makeDump(sd_journal *context)
{
    auto cursorExpected = contextual_owning_ptr_call<char>(sd_journal_get_cursor, context, std::free);
    if (cursorExpected.ret != 0) {
        qFatal("Failed to get entry cursor");
        return std::nullopt;
    }

    Coredump::EntriesHash entries;
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
            qWarning() << "this entry looks funny it has no separating = character" << entry;
            continue;
        }

        const QByteArray key = entry.left(offset);
        if (key == QByteArrayLiteral("COREDUMP")) {
            // The literal COREDUMP= entry is the actual core when configured for journal storage in coredump.conf.
            // Synthesize a filename instead so we can use the same validity checks for all storage types.
            entries.insert(Coredump::keyFilename(), QByteArrayLiteral("/dev/null"));
            continue;
        }

        const QByteArray value = entry.mid(offset + 1);

        // Always add to raw data, they get serialized back into the INI file for drkonqi.
        entries.insert(key, value);
    }

    return std::make_optional<Coredump>(cursorExpected.value.get(), entries);
}

CoredumpWatcher::CoredumpWatcher(std::unique_ptr<sd_journal> context_, QString bootId_, const QString &instance_, QObject *parent)
    : QObject(parent)
    , context(std::move(context_))
    , bootId(std::move(bootId_))
    , instance(instance_)
    , instanceFilter(QStringLiteral("systemd-coredump@%1").arg(instance_))
{
}

void CoredumpWatcher::processLog()
{
    int i = 0;
    while (sd_journal_next(context.get()) > 0) {
        ++i;
        const auto optionalDump = makeDump(context.get());
        if (!optionalDump.has_value()) {
            qWarning() << "Failed to make a dump :O";
            continue;
        }

        const Coredump &dump = optionalDump.value();
        if (!dump.systemd_unit.startsWith(instanceFilter)) {
            // Older systemds have trouble templating a correct instance. We only
            // perform a startsWith check here, but will filter more aggressively
            // whenever possible via the constructor.
            continue;
        }
        if (dump.exe.isEmpty() && dump.filename.isEmpty()) {
            qDebug() << "Entry doesn't look like a dump. This may have been a vaccum run. Nothing to process.";
            // Do not finish here. Vaccum log entires are created from real coredump processes. We should eventually
            // find a dump.
            continue;
        }

        qDebug() << dump.exe << dump.pid << dump.filename;

        Q_EMIT newDump(dump);

        constexpr int maximumInBatch = 128; // give the event loop a chance to do other stuff as well
        if (i >= maximumInBatch) {
            // reschedule run
            QMetaObject::invokeMethod(this, &CoredumpWatcher::processLog, Qt::QueuedConnection);
            return;
        }
    }
    Q_EMIT atLogEnd();
}

void CoredumpWatcher::errnoError(const QString &msg, int err)
{
    Q_EMIT error(msg + QStringLiteral(": (%1) ").arg(QString::number(err)) + QString::fromLocal8Bit(strerror(err)));
}

void CoredumpWatcher::start()
{
    Q_ASSERT(context);

    sd_journal_flush_matches(context.get()); // reset match
    if (sd_journal_add_match(context.get(), "SYSLOG_IDENTIFIER=systemd-coredump", 0) != 0) {
        Q_EMIT error(QStringLiteral("Failed to install id match"));
        return;
    }

    if (!bootId.isEmpty()) {
        const QString bootIdMatch = QStringLiteral("_BOOT_ID=%1").arg(bootId);
        if (sd_journal_add_match(context.get(), qPrintable(bootIdMatch), 0) != 0) {
            Q_EMIT error(QStringLiteral("Failed to install boot id match"));
            return;
        }
    }

    if (!instance.isEmpty()) {
        if (instance.count(QLatin1Char('-')) >= 2) {
            // older systemds have a bug where %I doesn't actually expand correctly and only contains the first element.
            // This prevents us from matching through sd API. Instead processLog will filter based on the instance
            // information prefix. It's still unique enough.
            // Auto-generated instance names are of the form
            // $iid-$pid-$uid where iid is a growing instance id number.
            // Additionally we'll filter by chrono proximity, iids that are too far in the past will be discarded.
            // This is because iid on its own isn't necessarily unique in the event that it wraps around whatever
            // integer limit it has.
            if (sd_journal_add_match(context.get(), qPrintable(QStringLiteral("_SYSTEMD_UNIT=%1.service").arg(instanceFilter)), 0) != 0) {
                Q_EMIT error(QStringLiteral("Failed to install unit match"));
                return;
            }
        }
    }

    for (const auto &match : matches) {
        if (sd_journal_add_match(context.get(), qUtf8Printable(match), 0) != 0) {
            Q_EMIT error(u"Failed to install custom match: %1"_s.arg(match));
            return;
        }
    }

    const int fd = sd_journal_get_fd(context.get());
    if (fd < 0) {
        errnoError(QStringLiteral("Failed to get listening socket"), -fd);
        return;
    }

    notifier = std::make_unique<QSocketNotifier>(fd, QSocketNotifier::Read);
    connect(notifier.get(), &QSocketNotifier::activated, this, [this] {
        if (sd_journal_process(context.get()) != SD_JOURNAL_APPEND) {
            return;
        }
        processLog();
    });

    if (int ret = sd_journal_seek_head(context.get()); ret != 0) {
        errnoError(QStringLiteral("Failed to go to tail"), -fd);
        return;
    }
    // Make sure to read whatever we have pending on next loop.
    QMetaObject::invokeMethod(this, &CoredumpWatcher::processLog);
}

void CoredumpWatcher::addMatch(const QString &str)
{
    matches.push_back(str);
}

#include "moc_coredumpwatcher.cpp"
