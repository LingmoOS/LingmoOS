/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofe.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "filestorage.h"
#include "memorycalendar.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QTimeZone>

using namespace KCalendarCore;

int main(int argc, char **argv)
{
    qputenv("TZ", "GMT");

    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("verbose"), QStringLiteral("Verbose output")));

    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("Name of input file"));

    parser.addPositionalArgument(QStringLiteral("output"), QStringLiteral("optional name of output file for the recurrence dates"));

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("testrecurson"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    parser.process(app);

    const QStringList parsedArgs = parser.positionalArguments();

    if (parsedArgs.isEmpty()) {
        parser.showHelp();
    }

    QString input = parsedArgs[0];

    QTextStream *outstream = nullptr;
    QString fn;
    if (parsedArgs.count() > 1) {
        fn = parsedArgs[1];
    }
    QFile outfile(fn);
    if (!fn.isEmpty() && outfile.open(QIODevice::WriteOnly)) {
        outstream = new QTextStream(&outfile);
    }

    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    FileStorage store(cal, input);
    if (!store.load()) {
        return 1;
    }
    QString tz = cal->nonKDECustomProperty("X-LibKCal-Testsuite-OutTZ");
    const auto viewZone = tz.isEmpty() ? cal->timeZone() : QTimeZone(tz.toUtf8());

    const Incidence::List inc = cal->incidences();
    for (const Incidence::Ptr &incidence : inc) {
        //     qDebug() << " ->" << incidence->summary() << "<-";

        //     incidence->recurrence()->dump();

        QDate dt(1996, 7, 1);
        if (outstream) {
            // Output to file for testing purposes
            int nr = 0;
            while (dt.year() <= 2020 && nr <= 500) {
                if (incidence->recursOn(dt, viewZone)) {
                    (*outstream) << dt.toString(Qt::ISODate) << '\n';
                    nr++;
                }
                dt = dt.addDays(1);
            }
        } else {
            dt = QDate(2005, 1, 1);
            while (dt.year() < 2007) {
                if (incidence->recursOn(dt, viewZone)) {
                    qDebug() << dt.toString(Qt::ISODate);
                }
                dt = dt.addDays(1);
            }
        }
    }

    delete outstream;
    outfile.close();
    return 0;
}
