/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filestorage.h"
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
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("verbose"), QStringLiteral("Verbose output")));
    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("Name of input file"));
    parser.addPositionalArgument(QStringLiteral("output"), QStringLiteral("Name of output file"));

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("testvcalexport"));
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

    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));
    FileStorage instore(cal, input);

    if (!instore.load()) {
        return 1;
    }

    FileStorage outstore(cal, output, new VCalFormat);
    if (!outstore.save()) {
        return 1;
    }

    return 0;
}
