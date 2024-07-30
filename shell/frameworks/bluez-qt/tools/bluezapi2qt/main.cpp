/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "BluezApiParser.h"
#include "CppGenerator.h"
#include "XmlGenerator.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("bluezapi2qt"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));

    // Add command line parsing
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Generates D-BUS object introspection XML files out of BlueZ D-Bus API\n"
                       "description files (*-api.txt)."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("<bluez>-api.txt"), QStringLiteral("BlueZ D-Bus API description file to use."));
    QCommandLineOption deprecatedOption(QStringList() << QStringLiteral("d") << QStringLiteral("deprecated"),
                                        QStringLiteral("Generate deprecated methods/properties"));
    parser.addOption(deprecatedOption);
    QCommandLineOption experimentalOption(QStringList() << QStringLiteral("e") << QStringLiteral("experimental"),
                                          QStringLiteral("Generate experimental methods/properties"));
    parser.addOption(experimentalOption);
    QCommandLineOption optionalOption(QStringList() << QStringLiteral("o") << QStringLiteral("optional"),
                                      QStringLiteral("Generate optional methods/properties"));
    parser.addOption(optionalOption);
    QCommandLineOption xmlOption(QStringList() << QStringLiteral("x") << QStringLiteral("xml"),
                                 QStringLiteral("Generate D-Bus object introspection XML files"));
    parser.addOption(xmlOption);
    QCommandLineOption cppOption(QStringList() << QStringLiteral("c") << QStringLiteral("cpp"), QStringLiteral("Generate D-Bus interface adaptor C++ files"));
    parser.addOption(cppOption);
    QCommandLineOption authorOption(QStringList() << QStringLiteral("a") << QStringLiteral("author"),
                                    QStringLiteral("Author for copyright header in C++ files"),
                                    QStringLiteral("author"));
    parser.addOption(authorOption);
    QCommandLineOption yearOption(QStringList() << QStringLiteral("y") << QStringLiteral("year"),
                                  QStringLiteral("Year for copyright header in C++ files"),
                                  QStringLiteral("year"));
    parser.addOption(yearOption);
    parser.process(a);

    // Open file
    auto positionalArgs = parser.positionalArguments();
    if (positionalArgs.isEmpty()) {
        fputs(qPrintable(parser.helpText()), stderr);
        return 1;
    }
    auto fileName = positionalArgs.takeFirst();
    QFileInfo inputInfo(fileName);
    if (!inputInfo.exists() || !inputInfo.isFile() || !inputInfo.isReadable()) {
        qCritical() << "Cannot open file" << fileName;
        return 1;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open file" << file.fileName();
        return 1;
    }
    QTextStream stream(&file);

    // Parse file
    BluezApiParser bluezParser;
    if (!bluezParser.parse(stream)) {
        qCritical() << "Error parsing file" << file.fileName();
        return 1;
    }
    if (!bluezParser.finalize()) {
        qCritical() << "Error parsing file" << file.fileName();
        return 1;
    }

    if (parser.isSet(xmlOption)) {
        XmlGenerator::Config xmlConfig;
        xmlConfig.useOptional = parser.isSet(optionalOption);
        xmlConfig.useDeprecated = parser.isSet(deprecatedOption);
        xmlConfig.useExperimental = parser.isSet(experimentalOption);
        XmlGenerator xmlGenerator(xmlConfig);
        if (!xmlGenerator.generate(bluezParser)) {
            qCritical() << "Error generating xml";
            return 1;
        }
    }

    if (parser.isSet(cppOption)) {
        CppGenerator::Config cppConfig;
        cppConfig.author = parser.value(authorOption);
        cppConfig.year = parser.value(yearOption);
        CppGenerator cppGenerator(cppConfig);
        if (!cppGenerator.generate(bluezParser)) {
            qCritical() << "Error generating C++ files";
            return 1;
        }
    }

    return 0;
}
