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
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTimeZone>

using namespace KCalendarCore;

static QString dumpTime(const QDateTime &dt, const QTimeZone &viewZone);

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("verbose"), QStringLiteral("Verbose output")));
    parser.addPositionalArgument(QStringLiteral("input"), QStringLiteral("Name of input file"));
    parser.addPositionalArgument(QStringLiteral("output"), QStringLiteral("optional name of output file for the recurrence dates"));

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("testrecurrencenew"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    parser.process(app);

    const QStringList parsedArgs = parser.positionalArguments();

    if (parsedArgs.isEmpty()) {
        parser.showHelp();
    }

    QString input = parsedArgs[0];
    qDebug() << "Input file:" << input;

    QTextStream *outstream = nullptr;
    QString fn;
    if (parsedArgs.count() > 1) {
        fn = parsedArgs[1];
        qDebug() << "We have a file name given:" << fn;
    }
    QFile outfile(fn);
    if (!fn.isEmpty() && outfile.open(QIODevice::WriteOnly)) {
        qDebug() << "Opened output file!!!";
        outstream = new QTextStream(&outfile);
    }

    MemoryCalendar::Ptr cal(new MemoryCalendar(QTimeZone::utc()));

    QTimeZone viewZone;
    FileStorage store(cal, input);
    if (!store.load()) {
        return 1;
    }
    QString tz = cal->nonKDECustomProperty("X-LibKCal-Testsuite-OutTZ");
    if (!tz.isEmpty()) {
        viewZone = QTimeZone(tz.toUtf8());
    }

    const Incidence::List inc = cal->incidences();
    for (const Incidence::Ptr &incidence : inc) {
        qDebug() << "*+*+*+*+*+*+*+*+*+*";
        qDebug() << " ->" << incidence->summary() << "<-";

        incidence->recurrence()->dump();

        QDateTime dt;
        if (incidence->allDay()) {
            dt = incidence->dtStart().addDays(-1);
        } else {
            dt = incidence->dtStart().addSecs(-1);
        }
        int i = 0;
        if (outstream) {
            // Output to file for testing purposes
            while (dt.isValid() && i < 500) {
                ++i;
                dt = incidence->recurrence()->getNextDateTime(dt);
                if (dt.isValid()) {
                    (*outstream) << dumpTime(dt, viewZone) << '\n';
                }
            }
        } else {
            incidence->recurrence()->dump();
            // Output to konsole
            while (dt.isValid() && i < 10) {
                ++i;
                qDebug() << "-------------------------------------------";
                dt = incidence->recurrence()->getNextDateTime(dt);
                if (dt.isValid()) {
                    qDebug() << " *~*~*~*~ Next date is:" << dumpTime(dt, viewZone);
                }
            }
        }
    }

    delete outstream;
    outfile.close();
    return 0;
}

QString dumpTime(const QDateTime &dt, const QTimeZone &viewZone)
{
    if (!dt.isValid()) {
        return QString();
    }
    QDateTime vdt = viewZone.isValid() ? dt.toTimeZone(viewZone) : dt;
    QString format = QStringLiteral("yyyy-MM-ddThh:mm:ss t");
    if (viewZone.isValid()) {
        format += QStringLiteral(" '%1'").arg(QString::fromUtf8(viewZone.id()));
    }
    return vdt.toString(format);
}
