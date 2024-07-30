/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDBusConnection>
#include <QFile>
#include <QJsonDocument>
#include <QTimer>

#include <iostream>

#include "fakeserver.h"

int main(int argc, char **argv)
{
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(QStringLiteral("fakeserver"));

    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption cfgOption(QStringLiteral("cfg"), QStringLiteral("Config file"), QStringLiteral("FILE"));
    parser.addOption(cfgOption);
    parser.addHelpOption();
    parser.process(app);
    if (!parser.isSet(cfgOption)) {
        std::cout << "Missing option --cfg" << std::endl;
        parser.showHelp();
        return 0;
    }

    try {
        FakeServer server(parser.value(cfgOption));
        return app.exec();
    } catch (const FakeServerException &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}
