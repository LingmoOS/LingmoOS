/*
    SPDX-FileCopyrightText: 2023 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../isocodescache_p.h"

#include <QCommandLineParser>
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption codeOpt(QStringLiteral("code"), QStringLiteral("ISO code type to generate a cache for [3166-1, 3166-2]"), QStringLiteral("code"));
    parser.addOption(codeOpt);
    QCommandLineOption inputOpt(QStringLiteral("input"), QStringLiteral("Input ISO codes JSON file to generate the cache for."), QStringLiteral("input"));
    parser.addOption(inputOpt);
    QCommandLineOption outputOpt(QStringLiteral("output"), QStringLiteral("Generated cache file."), QStringLiteral("output"));
    parser.addOption(outputOpt);
    parser.addHelpOption();
    parser.process(app);

    const QString code = parser.value(codeOpt);
    const QString inputFile = parser.value(inputOpt);
    const QString outputFile = parser.value(outputOpt);

    if (code == QLatin1String("3166-1")) {
        IsoCodesCache::createIso3166_1Cache(inputFile, outputFile);
    } else if (code == QLatin1String("3166-2")) {
        IsoCodesCache::createIso3166_2Cache(inputFile, outputFile);
    } else {
        parser.showHelp();
        return 1;
    }

    return 0;
}
