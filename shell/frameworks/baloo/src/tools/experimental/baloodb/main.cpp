/*
    SPDX-FileCopyrightText: 2018 Michael Heidelbach <heidelbach@web.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "global.h"
#include "experimental/databasesanitizer.h"

#include <KAboutData>
#include <KLocalizedString>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTextStream>
#include <QList>
#include <QElapsedTimer>

using namespace Baloo;

struct Command {
    const QString name;
    const QString description;
    const QStringList args;
    const QStringList options;
};

const auto options = QList<QCommandLineOption>{
    QCommandLineOption{
        QStringList{QStringLiteral("i"), QStringLiteral("device-id")},
        i18n("Filter by device id."
        "\n0 (default) does not filter and everything is printed."
        "\nPositive numbers are including filters printing only the mentioned device id."
        "\nNegative numbers are excluding filters printing everything but the mentioned device id."
        "\nMay be given multiple times."),
        i18n("integer"),
        nullptr
    },
    QCommandLineOption{
        QStringList{QStringLiteral("m"), QStringLiteral("missing-only")},
        i18n("List only inaccessible entries.\nOnly applies to \"%1\"", QStringLiteral("list"))
    },
    QCommandLineOption{
        QStringList{QStringLiteral("u"), QStringLiteral("mounted-only")},
        i18n("Act only on item on mounted devices")
    },
    QCommandLineOption{
        QStringList{QStringLiteral("D"), QStringLiteral("dry-run")},
        i18n("Print results of a cleaning operation, but do not change anything."
        "\nOnly applies to \"%1\" command", QStringLiteral("clean"))
    }
};

const auto commands = std::vector<Command>{
    Command{
        QStringLiteral("list"),
        i18n("List database contents. Use a regular expression as argument to filter output"),
        QStringList{
            QStringLiteral("pattern")
        },
        QStringList{
            QStringLiteral("missing-only"),
            QStringLiteral("device-id")
        }
    },
    Command{
        QStringLiteral("devices"),
        i18n("List devices"),
        QStringList{},
        QStringList{QStringLiteral("missing-only")}
    },
    /*TODO:
    Command{
        QStringLiteral("check"),
        i18n("Check database contents. "
        "Beware this may take very long to execute"),
        QStringList{},
        QStringList{}
    },
    */
    Command{
        QStringLiteral("clean"),
        i18n("Remove stale database entries"),
        QStringList{
            QStringLiteral("pattern")
        },
        QStringList{
            QStringLiteral("dry-run"),
            QStringLiteral("device-id"),
            QStringLiteral("mounted-only")
        }
    }
};

const QStringList allowedCommands()
{
    QStringList names;
    for (const auto& c : commands) {
        names.append(c.name);
    }
    return names;
}
const QStringList getOptions(const QString& name)
{
    for (const auto& c : commands) {
        if (c.name == name) {
            return c.options;
        }
    }
    return QStringList();
}
QString createDescription()
{
    QStringList allowedcommands;
    for (const auto& c: commands) {
        auto options = getOptions(c.name);
        const QString optionStr = options.isEmpty()
            ? QString()
            : QStringLiteral(" [--%1]").arg(options.join(QLatin1String("] [--")));

        QString argumentStr;
        if (!c.args.isEmpty() ) {
            argumentStr = QStringLiteral(" [%1]").arg(c.args.join(QStringLiteral("] [")));
        }

        const QString commandStr = QStringLiteral("%1%2%3")
            .arg(c.name)
            .arg(optionStr)
            .arg(argumentStr);

        const QString str = QStringLiteral("%1 %2")
            .arg(commandStr, -58)
            .arg(c.description);

        allowedcommands.append(str);
    }
    const QString allCommandsStr = allowedcommands.join(QLatin1String("\n    "));
    return i18n("\n\nCommands:\n    %1", allCommandsStr);
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("baloodb"),
                         i18n("Baloo Database Sanitizer"),
                         PROJECT_VERSION,
                         i18n("The Baloo Database Lister & Sanitizer"),
                         KAboutLicense::GPL,
                         i18n("(c) 2018, Michael Heidelbach"));
    aboutData.addAuthor(i18n("Michael Heidelbach"), i18n("Maintainer"), QStringLiteral("ottwolt@gmail.com"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addOptions(options);
    parser.addPositionalArgument(QStringLiteral("command"),
        i18n("The command to execute"),
        allowedCommands().join(QLatin1Char('|'))
    );
    parser.addPositionalArgument(QStringLiteral("pattern"),
        i18nc("Command", "A regular expression applied to the URL of database items"
            "\nExample: %1"
            , "baloodb list '^/media/videos/series'"
        )
    );
    const QString warnExperiment = QStringLiteral(
        "===\nPlease note: This is an experimental tool. Command line switches or their meaning may change.\n===");

    parser.setApplicationDescription(warnExperiment + createDescription());
    parser.addVersionOption();
    parser.addHelpOption();

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        qDebug() << "No command";
        parser.showHelp(1);
    }

    auto args = parser.positionalArguments();
    auto command = args.at(0);
    args.removeFirst();

    if(!allowedCommands().contains(command)) {
        qDebug() << "Unknown command" << command;
        parser.showHelp(1);
    }

    const auto optNames = parser.optionNames();
    const auto allowedOptions = getOptions(command);

    QVector<qint64> deviceIds;
    for (const auto& dev : parser.values(QStringLiteral("device-id"))) {
        deviceIds.append(dev.toInt());
    }
    const DatabaseSanitizer::ItemAccessFilters accessFilter = (
        parser.isSet(QStringLiteral("missing-only"))
        ? DatabaseSanitizer::IgnoreAvailable
        : DatabaseSanitizer::IgnoreNone
    ) | (
        parser.isSet(QStringLiteral("mounted-only"))
        ? DatabaseSanitizer::IgnoreUnmounted
        : DatabaseSanitizer::IgnoreNone
    );
    const QString pattern = args.isEmpty()
        ? QString()
        : args.at(0);
    const QSharedPointer<QRegularExpression> urlFilter(pattern.isEmpty()
        ? nullptr
        : new QRegularExpression{pattern});

    auto db = globalDatabaseInstance();
    QTextStream err(stderr);
    QElapsedTimer timer;
    timer.start();

    if (command == QLatin1String("list")) {
        if (!db->open(Database::ReadOnlyDatabase)) {
            err << i18n("Baloo Index could not be opened") << endl;
            return 1;
        }
        DatabaseSanitizer san(db, Transaction::ReadOnly);
        err << i18n("Listing database contents...") << endl;
        san.printList(deviceIds, accessFilter, urlFilter);
    } else if (command == QLatin1String("devices")) {
        if (!db->open(Database::ReadOnlyDatabase)) {
            err << i18n("Baloo Index could not be opened") << endl;
            return 1;
        }
        DatabaseSanitizer san(db, Transaction::ReadOnly);
        err << i18n("Listing database contents...") << endl;
        san.printDevices(deviceIds, accessFilter);

    } else if (command == QLatin1String("clean")) {
        auto dbMode = Database::ReadWriteDatabase;
        if (!db->open(dbMode)) {
            err << i18n("Baloo Index could not be opened") << endl;
            return 1;
        }
        DatabaseSanitizer san(db, Transaction::ReadWrite);
        err << i18n("Removing stale database contents...") << endl;
        san.removeStaleEntries(deviceIds, accessFilter, parser.isSet(QStringLiteral("dry-run")), urlFilter);

    } else if (command == QLatin1String("check")) {
        parser.showHelp(1);
       /* TODO: After check methods are improved
            Database *db = globalDatabaseInstance();
            if (!db->open(Database::ReadOnlyDatabase)) {
                err << i18n("Baloo Index could not be opened") << endl;
                return 1;
            }
            Transaction tr(db, Transaction::ReadOnly);
            err << i18n("Checking file paths ... ") << endl;
            tr.checkFsTree();

            err << i18n("Checking postings ... ") << endl;
            tr.checkTermsDbinPostingDb();

            err << i18n("Checking terms ... ") << endl;
            tr.checkPostingDbinTermsDb();
        */
    }
    err << i18n("Elapsed: %1 secs", timer.nsecsElapsed() / 1000000000.0) << endl;

    return 0;
}
