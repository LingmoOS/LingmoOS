/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "configcommand.h"
#include "indexerconfig.h"

#include <QDir>
#include <QTextStream>
#include <QFileInfo>

#include <KLocalizedString>

using namespace Baloo;

/*
 * TODO: remove code duplication, we are performing similar operations
 * for excludeFolders, includeFolders and excludeFilters just using different
 * setters/getters. Figure out a way to unify the code while still keeping it
 * readable.
 */

namespace
{
QString normalizeTrailingSlashes(QString&& path)
{
    while (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    path += QLatin1Char('/');
    return path;
}
}

QString ConfigCommand::command()
{
    return QStringLiteral("config");
}

QString ConfigCommand::description()
{
    return i18n("Manipulate the Baloo configuration");
}

int ConfigCommand::exec(const QCommandLineParser& parser)
{
    QStringList args = parser.positionalArguments();
    args.removeFirst();

    QTextStream out(stdout);

    auto printCommand = [&out](const QString& command, const QString& description) {
        out << "  ";
        out.setFieldWidth(25);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << command;
        out.setFieldWidth(0);
        out.setFieldAlignment(QTextStream::AlignLeft);
        out << description << '\n';
    };

    const QString command = args.isEmpty() ? QStringLiteral("help") : args.takeFirst();
    if (command == QLatin1String("help")) {
        // Show help
        out << i18n("The config command can be used to manipulate the Baloo Configuration\n");
        out << i18n("Usage: balooctl config <command>\n\n");
        out << i18n("Possible Commands:\n");

        printCommand(QStringLiteral("add"), i18n("Add a value to config parameter"));
        printCommand(QStringLiteral("rm | remove"), i18n("Remove a value from a config parameter"));
        printCommand(QStringLiteral("list | ls | show"), i18n("Show the value of a config parameter"));
        printCommand(QStringLiteral("set"), i18n("Set the value of a config parameter"));
        printCommand(QStringLiteral("help"), i18n("Display this help menu"));
        return 0;
    }

    if (command == QLatin1String("list") || command == QLatin1String("ls") || command == QLatin1String("show")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be listed:\n\n");

            printCommand(QStringLiteral("hidden"), i18n("Controls if Baloo indexes hidden files and folders"));
            printCommand(QStringLiteral("contentIndexing"), i18n("Controls if Baloo indexes file content"));
            printCommand(QStringLiteral("includeFolders"), i18n("The list of folders which Baloo indexes"));
            printCommand(QStringLiteral("excludeFolders"), i18n("The list of folders which Baloo will never index"));
            printCommand(QStringLiteral("excludeFilters"), i18n("The list of filters which are used to exclude files"));
            printCommand(QStringLiteral("excludeMimetypes"), i18n("The list of mimetypes which are used to exclude files"));
            return 0;
        }

        IndexerConfig config;
        QString value = args.takeFirst();
        if (value.compare(QLatin1String("hidden"), Qt::CaseInsensitive) == 0) {
            if (config.indexHidden()) {
                out << "yes\n";
            } else {
                out << "no\n";
            }

            return 0;
        }

        if (value.compare(QStringLiteral("contentIndexing"), Qt::CaseInsensitive) == 0) {
            if (config.onlyBasicIndexing()) {
                out << "no\n";
            } else {
                out << "yes\n";
            }

            return 0;
        }

        auto printList = [&out](const QStringList& list) {
            for (const QString& item: list) {
                out << item << '\n';
            }
        };

        if (value.compare(QLatin1String("includeFolders"), Qt::CaseInsensitive) == 0) {
            printList(config.includeFolders());
            return 0;
        }

        if (value.compare(QLatin1String("excludeFolders"), Qt::CaseInsensitive) == 0) {
            printList(config.excludeFolders());
            return 0;
        }

        if (value.compare(QStringLiteral("excludeFilters"), Qt::CaseInsensitive) == 0) {
            printList(config.excludeFilters());
            return 0;
        }

        if (value.compare(QStringLiteral("excludeMimetypes"), Qt::CaseInsensitive) == 0) {
            printList(config.excludeMimetypes());
            return 0;
        }

        out << i18n("Config parameter could not be found\n");
        return 1;
    }

    if (command == QLatin1String("rm") || command == QLatin1String("remove")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be modified:\n\n");

            printCommand(QStringLiteral("includeFolders"), i18n("The list of folders which Baloo indexes"));
            printCommand(QStringLiteral("excludeFolders"), i18n("The list of folders which Baloo will never index"));
            printCommand(QStringLiteral("excludeFilters"), i18n("The list of filters which are used to exclude files"));
            printCommand(QStringLiteral("excludeMimetypes"), i18n("The list of mimetypes which are used to exclude files"));
            return 0;
        }

        IndexerConfig config;
        QString value = args.takeFirst();
        if (value.compare(QLatin1String("includeFolders"), Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided\n");
                return 1;
            }

            QString path = args.takeFirst().replace(QStringLiteral("$HOME"), QDir::homePath());
            path = normalizeTrailingSlashes(std::move(path));
            QStringList folders = config.includeFolders();
            if (!folders.contains(path)) {
                out << i18n("%1 is not in the list of include folders", path) << '\n';
                return 1;
            }

            folders.removeAll(path);
            config.setIncludeFolders(folders);

            return 0;
        }

        if (value.compare(QLatin1String("excludeFolders"), Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided\n");
                return 1;
            }

            QString path = args.takeFirst().replace(QStringLiteral("$HOME"), QDir::homePath());
            path = normalizeTrailingSlashes(std::move(path));
            QStringList folders = config.excludeFolders();
            if (!folders.contains(path)) {
                out << i18n("%1 is not in the list of exclude folders", path) << '\n';
                return 1;
            }

            folders.removeAll(path);
            config.setExcludeFolders(folders);

            return 0;
        }

        if (value.compare(QStringLiteral("excludeFilters"), Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A filter must be provided\n");
                return 1;
            }

            QStringList filters = config.excludeFilters();
            if (!filters.contains(args.first())) {
                out << i18n("%1 is not in list of exclude filters", args.first()) << '\n';
                return 1;
            }

            filters.removeAll(args.first());
            config.setExcludeFilters(filters);
            return 0;
        }

        if (value.compare(QStringLiteral("excludeMimetypes"), Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A mimetype must be provided\n");
                return 1;
            }

            QStringList mimetypes = config.excludeMimetypes();
            if (!mimetypes.contains(args.first())) {
                out << i18n("%1 is not in list of exclude mimetypes", args.first()) << '\n';
                return 1;
            }

            mimetypes.removeAll(args.first());
            config.setExcludeMimetypes(mimetypes);
            return 0;
        }

        out << i18n("Config parameter could not be found\n");
        return 1;
    }

    if (command == QLatin1String("add")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be modified:\n\n");

            printCommand(QStringLiteral("includeFolders"), i18n("The list of folders which Baloo indexes"));
            printCommand(QStringLiteral("excludeFolders"), i18n("The list of folders which Baloo will never index"));
            printCommand(QStringLiteral("excludeFilters"), i18n("The list of filters which are used to exclude files"));
            printCommand(QStringLiteral("excludeMimetypes"), i18n("The list of mimetypes which are used to exclude files"));
            return 0;
        }

        IndexerConfig config;
        QString value = args.takeFirst();
        if (value.compare(QLatin1String("includeFolders"), Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided\n");
                return 1;
            }

            auto fileInfo = QFileInfo(args.takeFirst());
            if (!fileInfo.exists()) {
                out << i18n("Path does not exist\n");
                return 1;
            }

            if (!fileInfo.isDir()) {
                out << i18n("Path is not a directory\n");
                return 1;
            }

            auto path = normalizeTrailingSlashes(fileInfo.absoluteFilePath());
            QStringList folders = config.includeFolders();
            if (folders.contains(path)) {
                out << i18n("%1 is already in the list of include folders", path) << '\n';
                return 1;
            }

            if (config.excludeFolders().contains(path)) {
                out << i18n("%1 is in the list of exclude folders", path) << '\n';
                return 1;
            }

            folders.append(path);
            config.setIncludeFolders(folders);

            return 0;
        }

        if (value.compare(QLatin1String("excludeFolders"), Qt::CaseInsensitive) == 0) {
            if (args.isEmpty()) {
                out << i18n("A folder must be provided\n");
                return 1;
            }

            auto fileInfo = QFileInfo(args.takeFirst());
            if (!fileInfo.exists()) {
                out << i18n("Path does not exist\n");
                return 1;
            }

            if (!fileInfo.isDir()) {
                out << i18n("Path is not a directory\n");
                return 1;
            }

            auto path = normalizeTrailingSlashes(fileInfo.absoluteFilePath());
            QStringList folders = config.excludeFolders();
            if (folders.contains(path)) {
                out << i18n("%1 is already in the list of exclude folders", path) << '\n';
                return 1;
            }

            if (config.includeFolders().contains(path)) {
                out << i18n("%1 is in the list of exclude folders", path) << '\n';
                return 1;
            }

            folders.append(path);
            config.setExcludeFolders(folders);

            return 0;
        }

        if (value.compare(QStringLiteral("excludeFilters"), Qt::CaseInsensitive) == 0) {
            if (args.empty()) {
                out << i18n("A filter must be provided\n");
                return 1;
            }

            QStringList filters = config.excludeFilters();
            if (filters.contains(args.first())) {
                out << i18n("Exclude filter is already in the list\n");
                return 1;
            }

            filters.append(args.first());
            config.setExcludeFilters(filters);

            return 0;
        }

        if (value.compare(QStringLiteral("excludeMimetypes"), Qt::CaseInsensitive) == 0) {
            if (args.empty()) {
                out << i18n("A mimetype must be provided\n");
                return 1;
            }

            QStringList mimetypes = config.excludeMimetypes();
            if (mimetypes.contains(args.first())) {
                out << i18n("Exclude mimetype is already in the list\n");
                return 1;
            }

            mimetypes.append(args.first());
            config.setExcludeMimetypes(mimetypes);

            return 0;
        }

        out << i18n("Config parameter could not be found\n");
        return 1;
    }

    if (command == QLatin1String("set")) {
        if (args.isEmpty()) {
            out << i18n("The following configuration options may be modified:\n\n");

            printCommand(QStringLiteral("hidden"), i18n("Controls if Baloo indexes hidden files and folders"));
            printCommand(QStringLiteral("contentIndexing"), i18n("Controls if Baloo indexes file content"));
            return 0;
        }

        IndexerConfig config;
        QString configParam = args.takeFirst();

        if (configParam == QLatin1String("hidden"))  {
            if (args.isEmpty()) {
                out << i18n("A value must be provided\n");
                return 1;
            }

            QString value = args.takeFirst();
            if (value.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("y"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("1")) == 0) {
                config.setIndexHidden(true);
                return 0;
            }

            if (value.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("n"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("no"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("0")) == 0) {
                config.setIndexHidden(false);
                return 0;
            }

            out << i18n("Invalid value\n");
            return 1;
        }

        if (configParam.compare(QStringLiteral("contentIndexing"), Qt::CaseInsensitive) == 0)  {
            if (args.isEmpty()) {
                out << i18n("A value must be provided\n");
                return 1;
            }

            QString value = args.takeFirst();
            if (value.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("y"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("1")) == 0) {
                config.setOnlyBasicIndexing(false);
                return 0;
            }

            if (value.compare(QLatin1String("false"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("n"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("no"), Qt::CaseInsensitive) == 0
                    || value.compare(QLatin1String("0")) == 0) {
                config.setOnlyBasicIndexing(true);
                return 0;
            }

            out << i18n("Invalid value\n");
            return 1;
        }

        out << i18n("Config parameter could not be found\n");
        return 1;
    }

    return 0;
}
