/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <ottwolt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "databasesanitizer.h"
#include "documenturldb.h"
#include "idutils.h"

#include <sys/sysmacros.h>

#include <KLocalizedString>
#include <QFileInfo>
#include <QStorageInfo>
#include <QDebug>

namespace Baloo
{

class DatabaseSanitizerImpl {
public:
    DatabaseSanitizerImpl(const Database& db, Transaction::TransactionType type)
    : m_transaction(new Transaction(db, type))
    {
    }

public:

    /**
    * \brief Basic info about database items
    */
    struct FileInfo {
        quint32 deviceId = 0;
        quint32 inode = 0;
        quint64 id = 0;
        bool isSymLink = false;
        bool accessible = true;
        QString url;
    };

    void printProgress(QTextStream& out, uint& cur, const uint max, const uint step) const
    {
        if (cur % step == 0) {
            out << QStringLiteral("%1%2\r").arg(100 * cur / max, 6).arg("%", -16);
            out.flush();
        }
        cur++;
    }

    /**
     * Summary of createList() actions
     */
    struct Summary {
        quint64 total = 0;  ///Count of all files
        quint64 ignored = 0;      ///Count of filtered out files
        quint64 accessible = 0;   ///Count of checked and accessible files
    };
    /**
    * Create a list of \a FileInfo items.
    *
    * \p deviceIDs filter by device ids. If the vector is empty no filtering is done
    * and every item is collected.
    * Positive numbers are including filters collecting only the mentioned device ids.
    * Negative numbers are excluding filters collecting everything but the mentioned device ids.
    *
    * \p accessFilter Flags to filter items by accessibility.
    *
    * \p urlFilter Filter result urls. Default is null = Collect everything.
    */
    QPair<QVector<FileInfo>, Summary> createList(
        const QVector<qint64>& deviceIds,
        const DatabaseSanitizer::ItemAccessFilters accessFilter,
        const QSharedPointer<QRegularExpression>& urlFilter
    ) const
    {
        Q_ASSERT(m_transaction);

        const auto docUrlDb = DocumentUrlDB(m_transaction->m_dbis.idTreeDbi,
                                            m_transaction->m_dbis.idFilenameDbi,
                                            m_transaction->m_txn);
        const auto map = docUrlDb.toTestMap();
        const auto keys = map.keys();
        QVector<FileInfo> result;
        uint max = map.size();
        uint i = 0;
        result.reserve(max);
        QVector<quint32> includeIds;
        QVector<quint32> excludeIds;
        for (qint64 deviceId : deviceIds) {
            if (deviceId > 0) {
                includeIds.append(deviceId);
            } else if (deviceId < 0) {
                excludeIds.append(-deviceId);
            }
        }
        Summary summary;
        summary.total = max;
        summary.ignored = max;
        QTextStream err(stderr);

        for (auto it = map.constBegin(), end = map.constEnd(); it != end; it++) {
            printProgress(err, i, max, 100);
            const quint64 id = it.key();
            const quint32 deviceId = idToDeviceId(id);
            if (!includeIds.isEmpty() && !includeIds.contains(deviceId)) {
                continue;
            } else if (excludeIds.contains(deviceId)) {
                continue;
            } else if (urlFilter && !urlFilter->match(it.value()).hasMatch()) {
                continue;
            }

            FileInfo info;
            info.deviceId = deviceId;
            info.inode = idToInode(id);
            info.url = QFile::decodeName(it.value());
            info.id = id;
            QFileInfo fileInfo(info.url);
            info.accessible = !info.url.isEmpty() && fileInfo.exists();

            if (info.accessible && (accessFilter & DatabaseSanitizer::IgnoreAvailable)) {
                continue;
            } else if (!info.accessible && (accessFilter & DatabaseSanitizer::IgnoreUnavailable)) {
                continue;
            }

            info.isSymLink = fileInfo.isSymLink();

            result.append(info);
            summary.ignored--;
            if (info.accessible) {
                summary.accessible++;
            }
        }
        return {result, summary};
    }

    QStorageInfo getStorageInfo(const quint32 id) {
        static QMap<quint32, QStorageInfo> storageInfos = []() {
            QMap<quint32, QStorageInfo> result;
            const auto volumes = QStorageInfo::mountedVolumes();
            for (const auto& vol : volumes) {
                const QByteArray rootPath = QFile::encodeName(vol.rootPath());
                const auto id = filePathToId(rootPath);
                const quint32 deviceId = idToDeviceId(id);
                // qDebug() << vol;
                result[deviceId] = vol;
            }
            return result;
        }();

        QStorageInfo info = storageInfos.value(id);
        return info;
    }

    QMap<quint32, bool> deviceFilters(QVector<FileInfo>& infos, const DatabaseSanitizer::ItemAccessFilters accessFilter)
    {
        QMap<quint32, bool> result;
        for (const auto& info : infos) {
            result[info.deviceId] = false;
        }

        for (auto it = result.begin(), end = result.end(); it != end; it++) {
            const auto storageInfo = getStorageInfo(it.key());
            it.value() = isIgnored(storageInfo, accessFilter);
        }
        return result;
    }

    bool isIgnored(const QStorageInfo& storageInfo, const DatabaseSanitizer::ItemAccessFilters accessFilter)
    {
        const bool mounted = storageInfo.isValid();
        if (mounted && (accessFilter & DatabaseSanitizer::IgnoreMounted)) {
            return true;
        } else if (!mounted && (accessFilter & DatabaseSanitizer::IgnoreUnmounted)) {
            return true;
        }

        if (storageInfo.fileSystemType() == QLatin1String("tmpfs")) {
            // Due to the volatility of device ids, an id known by baloo may
            // appear as mounted, but is not what baloo expects.
            // For example at indexing time 43 was the id of a smb share, but
            // at runtime 43 is the id of /run/media/<uid> when other users are
            // logged in. The latter have a type of 'tmpfs' and should be ignored.
            return true;
        }

        return false;
    }

    void removeDocument(const quint64 id) {
        m_transaction->removeDocument(id);
    }

    void commit() {
        m_transaction->commit();
    }

    void abort() {
        m_transaction->abort();
    }

private:
    Transaction* m_transaction;
};
}

using namespace Baloo;

DatabaseSanitizer::DatabaseSanitizer(const Database& db, Baloo::Transaction::TransactionType type)
    : m_pimpl(new DatabaseSanitizerImpl(db, type))
{
}

DatabaseSanitizer::DatabaseSanitizer(Database* db, Transaction::TransactionType type)
    : DatabaseSanitizer(*db, type)
{
}

DatabaseSanitizer::~DatabaseSanitizer()
{
    delete m_pimpl;
    m_pimpl = nullptr;
}

/**
* Create a list of \a FileInfo items and print it to stdout.
*
* \p deviceIDs filter by device ids. If the vector is empty no filtering is done
* and everything is printed.
* Positive numbers are including filters printing only the mentioned device ids.
* Negative numbers are excluding filters printing everything but the mentioned device ids.
*
* \p missingOnly Simulate purging operation. Only inaccessible items are printed.
*
* \p urlFilter Filter result urls. Default is null = Print everything.
*/
 void DatabaseSanitizer::printList(
    const QVector<qint64>& deviceIds,
    const ItemAccessFilters accessFilter,
    const QSharedPointer<QRegularExpression>& urlFilter)
{
    auto listResult = m_pimpl->createList(deviceIds, accessFilter, urlFilter);
    const auto sep = QLatin1Char(' ');
    QTextStream out(stdout);
    QTextStream err(stderr);
    for (const auto& info: listResult.first) {
        out << QStringLiteral("%1").arg(info.accessible ? "+" : "!")
        << sep << QStringLiteral("device: %1").arg(info.deviceId)
        << sep << QStringLiteral("inode: %1").arg(info.inode)
        << sep << QStringLiteral("url: %1").arg(info.url)
        << endl;
    }

    const auto& summary = listResult.second;
    if (accessFilter & IgnoreAvailable) {
        err << i18n("Total: %1, Inaccessible: %2",
                    summary.total,
                    summary.total - (summary.ignored + summary.accessible)) << endl;
    } else {
        err << i18n("Total: %1, Ignored: %2, Accessible: %3, Inaccessible: %4",
                    summary.total,
                    summary.ignored,
                    summary.accessible,
                    summary.total - (summary.ignored + summary.accessible)) << endl;
    }
}

void DatabaseSanitizer::printDevices(const QVector<qint64>& deviceIds, const ItemAccessFilters accessFilter)
{
    auto infos = m_pimpl->createList(deviceIds, accessFilter, nullptr);

    QMap<quint32, quint64> useCount;
    for (const auto& info : infos.first) {
        useCount[info.deviceId]++;
    }

    const auto sep = QLatin1Char(' ');
    QTextStream out(stdout);
    QTextStream err(stderr);
    int matchCount = 0;
    for (auto it = useCount.cbegin(); it != useCount.cend(); it++) {
        auto id = it.key();
        auto info = m_pimpl->getStorageInfo(id);
        auto mounted = info.isValid();
        if (info.fileSystemType() == QLatin1String("tmpfs")) {
            continue;
        } else if (mounted && (accessFilter & IgnoreMounted)) {
            continue;
        } else if (!mounted && (accessFilter & IgnoreUnmounted)) {
            continue;
        }
        matchCount++;
        // TODO coloring would be nice, but "...|grep '^!'" does not work with it.
        // out << QStringLiteral("%1").arg(dev.mounted ? "+" : "\033[1;31m!")
        // Can be done, see: https://code.qt.io/cgit/qt/qtbase.git/tree/src/corelib/global/qlogging.cpp#n263
        out << QStringLiteral("%1").arg(mounted ? "+" : "!")
            << sep << QStringLiteral("device:%1").arg(id)
            << sep << QStringLiteral("[%1:%2]")
                .arg(major(id), 4, 16, QLatin1Char('0'))
                .arg(minor(id), 4, 16, QLatin1Char('0'))
            << sep << QStringLiteral("indexed-items:%1").arg(it.value());

        if (mounted) {
            out
                << sep << QStringLiteral("fstype:%1").arg(info.fileSystemType().toPercentEncoding().constData())
                << sep << QStringLiteral("device:%1").arg(info.device().constData())
                << sep << QStringLiteral("path:%1").arg(info.rootPath())
            ;
        }
        // TODO: see above
        // out << QStringLiteral("\033[0m") << endl;
        out << endl;
    }

    err << i18n("Found %1 matching in %2 devices", matchCount, useCount.size()) << endl;
}

void DatabaseSanitizer::removeStaleEntries(const QVector<qint64>& deviceIds,
    const DatabaseSanitizer::ItemAccessFilters accessFilter,
    const bool dryRun,
    const QSharedPointer<QRegularExpression>& urlFilter)
{
    auto listResult = m_pimpl->createList(deviceIds, IgnoreAvailable, urlFilter);

    const auto ignoredDevices = m_pimpl->deviceFilters(listResult.first, accessFilter);

    const auto sep = QLatin1Char(' ');
    auto& summary = listResult.second;
    QTextStream out(stdout);
    QTextStream err(stderr);
    for (const auto& info: listResult.first) {
        if (ignoredDevices[info.deviceId] == true) {
            summary.ignored++;
        } else {
            if (info.isSymLink) {
                out << i18n("IgnoredSymbolicLink:");
                summary.ignored++;
            } else {
                m_pimpl->removeDocument(info.id);
                out << i18n("Removing:");
            }
            out << sep << QStringLiteral("device: %1").arg(info.deviceId)
                << sep << QStringLiteral("inode: %1").arg(info.inode)
                << sep << QStringLiteral("url: %1").arg(info.url)
                << endl;
        }
    }
    if (dryRun) {
        m_pimpl->abort();
    } else {
        m_pimpl->commit();
    }
    Q_ASSERT(summary.accessible == 0);
    err << i18nc("numbers", "Removed: %1, Total: %2, Ignored: %3",
                 summary.total - summary.ignored,
                 summary.total,
                 summary.ignored)
        << endl;
}
