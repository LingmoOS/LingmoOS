/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AddPrinter.h"
#include <config.h>

#include <kde-add-printer_log.h>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

int main(int argc, char **argv)
{
    AddPrinter app(argc, argv);
    app.setOrganizationDomain(QLatin1String("org.kde"));

    KAboutData about(QLatin1String("kde-add-printer"),
                     i18n("Add Printer"),
                     QLatin1String(PM_VERSION),
                     i18n("Tool for adding new printers"),
                     KAboutLicense::GPL,
                     i18n("(C) 2010-2018 Daniel Nicoletti"));

    about.addAuthor(QLatin1String("Daniel Nicoletti"), QString(), QLatin1String("dantti12@gmail.com"));
    about.addAuthor(QStringLiteral("Lukáš Tinkl"), i18n("Port to Qt 5 / Plasma 5"), QStringLiteral("ltinkl@redhat.com"));
    about.addAuthor(QStringLiteral("Mike Noe"), i18n("Port to Qt 6 / Plasma 6"), QStringLiteral("noeerover@gmail.com"));
    KAboutData::setApplicationData(about);

    qCWarning(PM_ADD_PRINTER) << i18n("NOTE: This application is deprecated and will be removed from Plasma in a future release.");

    QCommandLineParser parser;
    about.setupCommandLine(&parser);

    QCommandLineOption parentWindowOpt({QLatin1String("w"), QLatin1String("parent-window")}, i18n("Parent Window ID"), QLatin1String("wid"));
    parser.addOption(parentWindowOpt);

    QCommandLineOption addPrinterOpt(QLatin1String("add-printer"), i18n("Add a new printer"));
    parser.addOption(addPrinterOpt);

    QCommandLineOption addClassOpt(QLatin1String("add-class"), i18n("Add a new printer class"));
    parser.addOption(addClassOpt);

    QCommandLineOption changePpdOpt(QLatin1String("change-ppd"), i18n("Changes the PPD of a given printer"), QLatin1String("printer-name"));
    parser.addOption(changePpdOpt);

    QCommandLineOption newPrinterDevOpt(QLatin1String("new-printer-from-device"),
                                        i18n("Changes the PPD of a given printer/deviceid"),
                                        QLatin1String("printername/deviceid"));
    parser.addOption(newPrinterDevOpt);

    parser.process(app);
    about.processCommandLine(&parser);

    qulonglong wid = 0;
    if (parser.isSet(parentWindowOpt)) {
        wid = parser.value(parentWindowOpt).toULongLong();
    }

    if (parser.isSet(addPrinterOpt)) {
        app.addPrinter(wid);
    } else if (parser.isSet(addClassOpt)) {
        app.addClass(wid);
    } else if (parser.isSet(changePpdOpt)) {
        app.changePPD(wid, parser.value(changePpdOpt));
    } else if (parser.isSet(newPrinterDevOpt)) {
        const QString value = parser.value(newPrinterDevOpt);
        const QStringList values = value.split(QLatin1String("/"));
        if (values.size() == 2) {
            app.newPrinterFromDevice(wid, values.first(), values.last());
        } else {
            qCWarning(PM_ADD_PRINTER) << i18n("The expected input should be printer/deviceid");
            exit(EXIT_FAILURE);
        }
    } else {
        parser.showHelp(EXIT_FAILURE);
    }

    return app.exec();
}
