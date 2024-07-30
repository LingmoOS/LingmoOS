/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filestorage.h"
#include "icalformat.h"
#include "memorycalendar.h"
#include "vcalformat.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QTimeZone>

using namespace KCalendarCore;

int main(int argc, char **argv)
{
    qputenv("TZ", "GMT");
    QHashSeed::setDeterministicGlobalSeed(); // Disable QHash randomness

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("verbose"), QStringLiteral("Verbose output")));
    parser.addPositionalArgument(QStringLiteral("source"), QStringLiteral("Source file to copy."));
    parser.addPositionalArgument(QStringLiteral("destination"), QStringLiteral("Destination directory."));

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("readandwrite"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    parser.process(app);

    const QStringList parsedArgs = parser.positionalArguments();
    if (parsedArgs.count() != 2) {
        parser.showHelp();
    }

    QString input = parsedArgs[0];
    QString output = parsedArgs[1];

    QFileInfo outputFileInfo(output);
    output = outputFileInfo.absoluteFilePath();

    qDebug() << "Input file:" << input;
    qDebug() << "Output file:" << output;

    if (outputFileInfo.fileName() == QLatin1String("KOrganizer_3.1.ics.ical.out")
        || outputFileInfo.fileName() == QLatin1String("KOrganizer_3.2.ics.ical.out")) {
        return 0;
    }

    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage instore(cal, input);

    if (!instore.load()) {
        qDebug() << "DAMN";
        return 1;
    }

    FileStorage outstore(cal, output);
    if (!outstore.save()) {
        return 1;
    }

    return 0;
}
