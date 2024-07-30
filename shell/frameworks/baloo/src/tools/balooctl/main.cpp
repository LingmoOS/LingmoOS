/*
    SPDX-FileCopyrightText: 2012-2015 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>

#include <KAboutData>
#include <KLocalizedString>
#include <KFormat>
#include <QProcess>
#include <QTextStream>
#include <QFileInfo>
#include <QLocale>

#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include "global.h"
#include "database.h"
#include "transaction.h"
#include "databasesize.h"
#include "config.h"

#include "indexer.h"
#include "indexerconfig.h"
#include "idutils.h"
#include "fileindexerconfig.h"
#include "monitorcommand.h"
#include "schedulerinterface.h"
#include "maininterface.h"
#include "indexerstate.h"
#include "configcommand.h"
#include "statuscommand.h"

using namespace Baloo;

void start()
{
    const QString exe = QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF "/baloo_file");
    QProcess::startDetached(exe, QStringList());
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    KAboutData aboutData(QStringLiteral("baloo"), i18n("balooctl"), QStringLiteral(PROJECT_VERSION));
    aboutData.addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("command"), i18n("The command to execute"));

    parser.addPositionalArgument(QStringLiteral("status"), i18n("Print the status of the indexer"));
    parser.addPositionalArgument(QStringLiteral("enable"), i18n("Enable the file indexer"));
    parser.addPositionalArgument(QStringLiteral("disable"), i18n("Disable the file indexer"));
    parser.addPositionalArgument(QStringLiteral("purge"), i18n("Remove the index database"));
    parser.addPositionalArgument(QStringLiteral("suspend"), i18n("Suspend the file indexer"));
    parser.addPositionalArgument(QStringLiteral("resume"), i18n("Resume the file indexer"));
    parser.addPositionalArgument(QStringLiteral("check"), i18n("Check for any unindexed files and index them"));
    parser.addPositionalArgument(QStringLiteral("index"), i18n("Index the specified files"));
    parser.addPositionalArgument(QStringLiteral("clear"), i18n("Forget the specified files"));
    parser.addPositionalArgument(QStringLiteral("config"), i18n("Modify the Baloo configuration"));
    parser.addPositionalArgument(QStringLiteral("monitor"), i18n("Monitor the file indexer"));
    parser.addPositionalArgument(QStringLiteral("indexSize"), i18n("Display the disk space used by index"));
    parser.addPositionalArgument(QStringLiteral("failed"), i18n("Display files which could not be indexed"));

    QString statusFormatDescription = i18nc("Format to use for status command, %1|%2|%3 are option values, %4 is a CLI command",
                                            "Output format <%1|%2|%3>.\nThe default format is \"%1\".\nOnly applies to \"%4\"",
                                                QStringLiteral("multiline"),
                                                QStringLiteral("json"),
                                                QStringLiteral("simple"),
                                                QStringLiteral("balooctl status <file>"));
    parser.addOption({{QStringLiteral("f"), QStringLiteral("format")},
                     statusFormatDescription, i18n("format"), QStringLiteral("multiline")});

    parser.addVersionOption();
    parser.addHelpOption();

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
    }

    QTextStream out(stdout);

    QString command = parser.positionalArguments().first();

    org::kde::baloo::main mainInterface(QStringLiteral("org.kde.baloo"),
                                                QStringLiteral("/"),
                                                QDBusConnection::sessionBus());

    org::kde::baloo::scheduler schedulerinterface(QStringLiteral("org.kde.baloo"),
                                        QStringLiteral("/scheduler"),
                                        QDBusConnection::sessionBus());

    if (command == QLatin1String("config")) {
        ConfigCommand command;
        return command.exec(parser);
    }

    if (command == QLatin1String("status")) {
        StatusCommand commandStatus;
        return commandStatus.exec(parser);
    }

    if (command == QLatin1String("enable") || command == QLatin1String("disable")) {
        bool isEnabled = false;
        if (command == QLatin1String("enable")) {
            isEnabled = true;
        }
        else if (command == QLatin1String("disable")) {
            isEnabled = false;
        }

        IndexerConfig cfg;
        cfg.setFileIndexingEnabled(isEnabled);

        if (isEnabled) {
            bool running = mainInterface.isValid();
            if (running) {
                out << "File Indexer already running\n";
            } else {
                out << "Enabling and starting the File Indexer\n";
                start();
            }
        } else {
            out << "Disabling and stopping the File Indexer\n";

            mainInterface.quit();
        }

        return 0;
    }

    if (command == QLatin1String("purge")) {
        bool running = mainInterface.isValid();

        if (running) {
            mainInterface.quit();
            out << "Stopping the File Indexer ...";
            for (int i = 5 * 60; i; --i) {
                QCoreApplication::processEvents();
                if (!mainInterface.isValid()) {
                    break;
                }
                out << "." << Qt::flush;
                QThread::msleep(200);
            }
            if (!mainInterface.isValid()) {
                out << " - done\n";
            } else {
                out << " - failed to stop!\n";
                return 1;
            }
        }

        const QString path = fileIndexDbPath() + QStringLiteral("/index");
        QFile(path).remove();
        out << "Deleted the index database\n";

        if (running) {
            start();
            out << "Restarting the File Indexer\n";
        }

        return 0;
    }

    if (command == QLatin1String("suspend")) {
        schedulerinterface.suspend();
        out << "File Indexer suspended\n";
        return 0;
    }

    if (command == QLatin1String("resume")) {
        schedulerinterface.resume();
        out << "File Indexer resumed\n";
        return 0;
    }

    if (command == QLatin1String("check")) {
        schedulerinterface.checkUnindexedFiles();
        out << "Started search for unindexed files\n";
        return 0;
    }

    if (command == QLatin1String("index")) {
        if (parser.positionalArguments().size() < 2) {
            out << "Please enter a filename to index\n";
            return 1;
        }

        Database *db = globalDatabaseInstance();
        if (!db->open(Database::ReadWriteDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadWrite);

        for (int i = 1; i < parser.positionalArguments().size(); ++i) {
            const QString url = QFileInfo(parser.positionalArguments().at(i)).absoluteFilePath();
            quint64 id = filePathToId(QFile::encodeName(url));
            if (id == 0) {
                out << "Could not stat file: " << url << '\n';
                continue;
            }
            if (tr.inPhaseOne(id))  {
                out << "Skipping: " << url << " Reason: Already scheduled for indexing\n";
                continue;
            }
            if (!tr.documentData(id).isEmpty()) {
                out << "Skipping: " << url << " Reason: Already indexed\n";
                continue;
            }
            Indexer indexer(url, &tr);
            out << "Indexing " << url << '\n';
            indexer.index();
        }
        tr.commit();
        out << "File(s) indexed\n";

        return 0;
    }

    if (command == QLatin1String("clear")) {
        if (parser.positionalArguments().size() < 2) {
            out << "Please enter a filename to index\n";
            return 1;
        }

        Database *db = globalDatabaseInstance();
        if (!db->open(Database::ReadWriteDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadWrite);

        for (int i = 1; i < parser.positionalArguments().size(); ++i) {
            const QString url = QFileInfo(parser.positionalArguments().at(i)).absoluteFilePath();
            quint64 id = filePathToId(QFile::encodeName(url));
            if (id == 0) {
                id = tr.documentId(QFile::encodeName(url));
                if (id == 0) {
                    out << "File not found on filesystem or in DB: " << url << '\n';
                    continue;
                } else {
                    out << "File has been deleted, clearing from DB: " << url << '\n';
                }
            } else {
                out << "Clearing " << url << '\n';
            }

            tr.removeDocument(id);
        }
        tr.commit();
        out << "File(s) cleared\n";

        return 0;
    }

    if (command == QLatin1String("failed")) {
        Database *db = globalDatabaseInstance();
        if (!db->open(Database::ReadOnlyDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadOnly);

        const quint64 limit = 128;
        const QVector<quint64> failedIds = tr.failedIds(limit);
        if (failedIds.isEmpty()) {
            out << "All Files were indexed successfully\n";
            return 0;
        }

        out << "The following files could not be indexed:\n";
        for (auto id : failedIds) {
            out << tr.documentUrl(id) << '\n';
        }
        if (failedIds.size() == limit) {
            out << "... list truncated\n";
        }
        return 0;
    }

    if (command == QLatin1String("indexSize")) {
        Database *db = globalDatabaseInstance();
        if (!db->open(Database::ReadOnlyDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        DatabaseSize size;
        {
            Transaction tr(db, Transaction::ReadOnly);
            size = tr.dbSize();
        }
        uint totalDataSize = size.expectedSize;

        KFormat format(QLocale::system());
        auto prFunc = [&](const QString& name, uint size) {
            out.setFieldWidth(20);
            out << name;
            out.setFieldWidth(0);
            out << ":";
            out.setFieldWidth(15);
            out << format.formatByteSize(size, 2);
            out.setFieldWidth(10);
            out << QString::number((100.0 * size / totalDataSize), 'f', 3);
            out.setFieldWidth(0);
            out << " %\n";
        };

        out << "File Size: " << format.formatByteSize(size.actualSize, 2) << "\n";
        out << "Used:      " << format.formatByteSize(totalDataSize, 2) << "\n\n";
        prFunc(QStringLiteral("PostingDB"), size.postingDb);
        prFunc(QStringLiteral("PositionDB"), size.positionDb);
        prFunc(QStringLiteral("DocTerms"), size.docTerms);
        prFunc(QStringLiteral("DocFilenameTerms"), size.docFilenameTerms);
        prFunc(QStringLiteral("DocXattrTerms"), size.docXattrTerms);
        prFunc(QStringLiteral("IdTree"), size.idTree);
        prFunc(QStringLiteral("IdFileName"), size.idFilename);
        prFunc(QStringLiteral("DocTime"), size.docTime);
        prFunc(QStringLiteral("DocData"), size.docData);
        prFunc(QStringLiteral("ContentIndexingDB"), size.contentIndexingIds);
        prFunc(QStringLiteral("FailedIdsDB"), size.failedIds);
        prFunc(QStringLiteral("MTimeDB"), size.mtimeDb);

        return 0;
    }

    if (command == QLatin1String("monitor")) {
        MonitorCommand mon;
        return mon.exec(parser);
    }

    /*
     TODO: Make separate executable
     if (command == QLatin1String("checkDb")) {
        Database *db = globalDatabaseInstance();
        if (!db->open(Database::ReadOnlyDatabase)) {
            out << "Baloo Index could not be opened\n";
            return 1;
        }

        Transaction tr(db, Transaction::ReadOnly);
        tr.checkPostingDbinTermsDb();
        tr.checkTermsDbinPostingDb();
        out << "Checking file paths .. "<< '\n';
        tr.checkFsTree();
        return 0;
    }
    */

    parser.showHelp(1);
    return 0;
}
