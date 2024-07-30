/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "statuscommand.h"
#include "indexerconfig.h"

#include "global.h"
#include "database.h"
#include "transaction.h"
#include "idutils.h"

#include "fileindexerinterface.h"
#include "schedulerinterface.h"
#include "maininterface.h"
#include "indexerstate.h"

#include <KLocalizedString>
#include <KFormat>

using namespace Baloo;

QString StatusCommand::command()
{
    return QStringLiteral("status");
}

QString StatusCommand::description()
{
    return i18n("Print the status of the Indexer");
}

class FileIndexStatus
{
public:
    enum class FileStatus : uint8_t {
        NonExisting,
        Directory,
        RegularFile,
        SymLink,
        Other,
    };
    enum class IndexStateReason : uint8_t {
        NoFileOrDirectory,
        ExcludedByPath, // FIXME - be more specific, requires changes to shouldBeIndexed(path)
        WaitingForIndexingBoth,
        WaitingForBasicIndexing,
        BasicIndexingDone,
        WaitingForContentIndexing,
        FailedToIndex,
        Done,
    };
    const QString m_filePath;
    FileStatus m_fileStatus;
    IndexStateReason m_indexState;
    uint32_t m_dataSize;
};

FileIndexStatus collectFileStatus(Transaction& tr, IndexerConfig&  cfg, const QString& file)
{
    using FileStatus = FileIndexStatus::FileStatus;
    using IndexStateReason = FileIndexStatus::IndexStateReason;

    bool onlyBasicIndexing = cfg.onlyBasicIndexing();

    const QFileInfo fileInfo = QFileInfo(file);
    const QString filePath = fileInfo.absoluteFilePath();
    quint64 id = filePathToId(QFile::encodeName(filePath));
    if (id == 0) {
        return FileIndexStatus{filePath, FileStatus::NonExisting, IndexStateReason::NoFileOrDirectory, 0};
    }

    FileStatus fileStatus = fileInfo.isSymLink() ? FileStatus::SymLink :
                            fileInfo.isFile() ? FileStatus::RegularFile :
                            fileInfo.isDir() ? FileStatus::Directory : FileStatus::Other;

    if (fileStatus == FileStatus::Other || fileStatus == FileStatus::SymLink) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::NoFileOrDirectory, 0};
    }

    if (!cfg.shouldBeIndexed(filePath)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::ExcludedByPath, 0};
    }

    if (onlyBasicIndexing || fileStatus == FileStatus::Directory) {
        if (!tr.hasDocument(id)) {
            return FileIndexStatus{filePath, fileStatus, IndexStateReason::WaitingForBasicIndexing, 0};
        } else {
            return FileIndexStatus{filePath, fileStatus, IndexStateReason::BasicIndexingDone, 0};
        }
    }

    // File && shouldBeIndexed && contentIndexing
    if (!tr.hasDocument(id)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::WaitingForIndexingBoth, 0};
    } else if (tr.inPhaseOne(id)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::WaitingForContentIndexing, 0};
    } else if (tr.hasFailed(id)) {
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::FailedToIndex, 0};
    } else {
        uint32_t size = tr.documentData(id).size();
        return FileIndexStatus{filePath, fileStatus, IndexStateReason::Done, size};
    }
}

void printMultiLine(Transaction& tr, IndexerConfig&  cfg, const QStringList& args) {
    using FileStatus = FileIndexStatus::FileStatus;
    using IndexStateReason = FileIndexStatus::IndexStateReason;

    QTextStream out(stdout);
    QTextStream err(stderr);

    const QMap<IndexStateReason, QString> basicIndexStateValue = {
        { IndexStateReason::NoFileOrDirectory,         i18n("File ignored") },
        { IndexStateReason::ExcludedByPath,            i18n("Basic Indexing: Disabled") },
        { IndexStateReason::WaitingForIndexingBoth,    i18n("Basic Indexing: Scheduled") },
        { IndexStateReason::WaitingForBasicIndexing,   i18n("Basic Indexing: Scheduled") },
        { IndexStateReason::BasicIndexingDone,         i18n("Basic Indexing: Done") },
        { IndexStateReason::WaitingForContentIndexing, i18n("Basic Indexing: Done") },
        { IndexStateReason::FailedToIndex,             i18n("Basic Indexing: Done") },
        { IndexStateReason::Done,                      i18n("Basic Indexing: Done") },
    };

    const QMap<IndexStateReason, QString> contentIndexStateValue = {
        { IndexStateReason::NoFileOrDirectory,         QString() },
        { IndexStateReason::ExcludedByPath,            QString() },
        { IndexStateReason::WaitingForIndexingBoth,    i18n("Content Indexing: Scheduled") },
        { IndexStateReason::WaitingForBasicIndexing,   QString() },
        { IndexStateReason::BasicIndexingDone,         QString() },
        { IndexStateReason::WaitingForContentIndexing, i18n("Content Indexing: Scheduled") },
        { IndexStateReason::FailedToIndex,             i18n("Content Indexing: Failed") },
        { IndexStateReason::Done,                      i18n("Content Indexing: Done") },
    };

    for (const auto& fileName : args) {
        const auto file = collectFileStatus(tr, cfg, fileName);

        if (file.m_fileStatus == FileStatus::NonExisting) {
            err << i18n("Ignoring non-existent file %1", file.m_filePath) << '\n';
            continue;
        }

        if (file.m_fileStatus == FileStatus::SymLink || file.m_fileStatus == FileStatus::Other) {
            err << i18n("Ignoring symlink/special file %1", file.m_filePath) << '\n';
            continue;
        }

        out << i18n("File: %1", file.m_filePath) << '\n';
        out << basicIndexStateValue[file.m_indexState] << '\n';
        const QString contentState = contentIndexStateValue[file.m_indexState];
        if (!contentState.isEmpty()) {
            out << contentState << '\n';
        }
    }
}

void printSimpleFormat(Transaction& tr, IndexerConfig&  cfg, const QStringList& args) {
    using FileStatus = FileIndexStatus::FileStatus;
    using IndexStateReason = FileIndexStatus::IndexStateReason;

    QTextStream out(stdout);
    QTextStream err(stderr);

    const QMap<IndexStateReason, QString> simpleIndexStateValue = {
        { IndexStateReason::NoFileOrDirectory,         QStringLiteral("No regular file or directory") },
        { IndexStateReason::ExcludedByPath,            QStringLiteral("Indexing disabled") },
        { IndexStateReason::WaitingForIndexingBoth,    QStringLiteral("Basic and Content indexing scheduled") },
        { IndexStateReason::WaitingForBasicIndexing,   QStringLiteral("Basic indexing scheduled") },
        { IndexStateReason::BasicIndexingDone,         QStringLiteral("Basic indexing done") },
        { IndexStateReason::WaitingForContentIndexing, QStringLiteral("Content indexing scheduled") },
        { IndexStateReason::FailedToIndex,             QStringLiteral("Content indexing failed") },
        { IndexStateReason::Done,                      QStringLiteral("Content indexing done") },
    };

    for (const auto& fileName : args) {
        const auto file = collectFileStatus(tr, cfg, fileName);

        if (file.m_fileStatus == FileStatus::NonExisting) {
            err << i18n("Ignoring non-existent file %1", file.m_filePath) << '\n';
            continue;
        }

        if (file.m_fileStatus == FileStatus::SymLink || file.m_fileStatus == FileStatus::Other) {
            err << i18n("Ignoring symlink/special file %1", file.m_filePath) << '\n';
            continue;
        }

        out << simpleIndexStateValue[file.m_indexState];
        out << ": " << file.m_filePath << '\n';
    }
}

void printJSON(Transaction& tr, IndexerConfig&  cfg, const QStringList& args)
{
    using FileStatus = FileIndexStatus::FileStatus;
    using IndexStateReason = FileIndexStatus::IndexStateReason;

    QJsonArray filesInfo;
    QTextStream err(stderr);

    const QMap<IndexStateReason, QString> jsonIndexStateValue = {
        { IndexStateReason::NoFileOrDirectory,         QStringLiteral("nofile") },
        { IndexStateReason::ExcludedByPath,            QStringLiteral("disabled") },
        { IndexStateReason::WaitingForIndexingBoth,    QStringLiteral("scheduled") },
        { IndexStateReason::WaitingForBasicIndexing,   QStringLiteral("scheduled") },
        { IndexStateReason::BasicIndexingDone,         QStringLiteral("done") },
        { IndexStateReason::WaitingForContentIndexing, QStringLiteral("scheduled") },
        { IndexStateReason::FailedToIndex,             QStringLiteral("failed") },
        { IndexStateReason::Done,                      QStringLiteral("done") },
    };

    const QMap<IndexStateReason, QString> jsonIndexLevelValue = {
        { IndexStateReason::NoFileOrDirectory,         QStringLiteral("nofile") },
        { IndexStateReason::ExcludedByPath,            QStringLiteral("none") },
        { IndexStateReason::WaitingForIndexingBoth,    QStringLiteral("content") },
        { IndexStateReason::WaitingForBasicIndexing,   QStringLiteral("basic") },
        { IndexStateReason::BasicIndexingDone,         QStringLiteral("basic") },
        { IndexStateReason::WaitingForContentIndexing, QStringLiteral("content") },
        { IndexStateReason::FailedToIndex,             QStringLiteral("content") },
        { IndexStateReason::Done,                      QStringLiteral("content") },
    };

    for (const auto& fileName : args) {
        const auto file = collectFileStatus(tr, cfg, fileName);

        if (file.m_fileStatus == FileStatus::NonExisting) {
            err << i18n("Ignoring non-existent file %1", file.m_filePath) << '\n';
            continue;
        }

        if (file.m_fileStatus == FileStatus::SymLink || file.m_fileStatus == FileStatus::Other) {
            err << i18n("Ignoring symlink/special file %1", file.m_filePath) << '\n';
            continue;
        }

        QJsonObject fileInfo;
        fileInfo[QStringLiteral("file")] = file.m_filePath;
        fileInfo[QStringLiteral("indexing")] = jsonIndexLevelValue[file.m_indexState];
        fileInfo[QStringLiteral("status")] = jsonIndexStateValue[file.m_indexState];

        filesInfo.append(fileInfo);
    }

    QJsonDocument json;
    json.setArray(filesInfo);
    QTextStream out(stdout);
    out << json.toJson(QJsonDocument::Indented);
}

int StatusCommand::exec(const QCommandLineParser& parser)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    const QStringList allowedFormats({QStringLiteral("simple"), QStringLiteral("json"), QStringLiteral("multiline")});
    const QString format = parser.value(QStringLiteral("format"));

    if (!allowedFormats.contains(format)) {
        err << i18n("Output format \"%1\" is invalid, use one of:\n", format);
        for (const auto& format : allowedFormats) {
            err << i18nc("bullet list item with output format", "- %1\n", format);
        }
        return 1;
    }

    IndexerConfig cfg;
    if (!cfg.fileIndexingEnabled()) {
        err << i18n("Baloo is currently disabled. To enable, please run %1\n", QStringLiteral("balooctl enable"));
        return 1;
    }

    Database *db = globalDatabaseInstance();
    if (!db->open(Database::ReadOnlyDatabase)) {
        err << i18n("Baloo Index could not be opened\n");
        return 1;
    }

    Transaction tr(db, Transaction::ReadOnly);

    QStringList args = parser.positionalArguments();
    args.pop_front();

    if (args.isEmpty()) {
        org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                    QStringLiteral("/"),
                                                    QDBusConnection::sessionBus());

        org::kde::baloo::scheduler schedulerinterface(QStringLiteral("org.kde.baloo"),
                                            QStringLiteral("/scheduler"),
                                            QDBusConnection::sessionBus());

        bool running = mainInterface.isValid();

        if (running) {
            org::kde::baloo::fileindexer indexerInterface(QStringLiteral("org.kde.baloo"),
            QStringLiteral("/fileindexer"),
            QDBusConnection::sessionBus());

            const QString currentFile = indexerInterface.currentFile();

            out << i18n("Baloo File Indexer is running\n");
            if (!currentFile.isEmpty()) {
                out << i18n("Indexer state: %1", stateString(IndexerState::ContentIndexing)) << '\n';
                out << i18nc("currently indexed file", "Indexing: %1", currentFile) << '\n';
            } else {
                out << i18n("Indexer state: %1", stateString(schedulerinterface.state())) << '\n';
            }
        }
        else {
            out << i18n("Baloo File Indexer is not running\n");
        }

        uint phaseOne = tr.phaseOneSize();
        uint total = tr.size();
        uint failed = tr.failedIds(100).size();

        out << i18n("Total files indexed: %1", total) << '\n';
        out << i18n("Files waiting for content indexing: %1", phaseOne) << '\n';
        out << i18n("Files failed to index: %1", failed) << '\n';

        const QString path = fileIndexDbPath();

        const QFileInfo indexInfo(path + QLatin1String("/index"));
        const auto size = indexInfo.size();
        KFormat format(QLocale::system());
        if (size) {
            out << i18n("Current size of index is %1", format.formatByteSize(size, 2)) << '\n';
        } else {
            out << i18n("Index does not exist yet\n");
        }
    } else if (format == allowedFormats[0]){
        printSimpleFormat(tr, cfg, args);
    } else if (format == allowedFormats[1]){
        printJSON(tr, cfg, args);
    } else {
        printMultiLine(tr, cfg, args);
    }

    return 0;
}
