/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/**
 * kpackagetool6 exit codes used in this program

    0 No error

    1 Unspecified error
    2 Plugin is not installed
    3 Plugin or package invalid
    4 Installation failed, see stderr for reason
    5 Could not find a suitable installer for package type
    6 No install option given
    7 Conflicting arguments supplied
    8 Uninstallation failed, see stderr for reason
    9 Failed to generate package hash

*/

#include <KLocalizedString>
#include <QCommandLineParser>

#include "kpackagetool.h"
#include "options.h"

int main(int argc, char **argv)
{
    QCommandLineParser parser;
    KPackage::PackageTool app(argc, argv, &parser);

    const QString description = i18n("KPackage Manager");
    const auto version = QStringLiteral("2.0");

    app.setApplicationVersion(version);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.setApplicationDescription(description);
    parser.addOptions({Options::hash(),
                       Options::global(),
                       Options::type(),
                       Options::install(),
                       Options::show(),
                       Options::upgrade(),
                       Options::list(),
                       Options::listTypes(),
                       Options::remove(),
                       Options::packageRoot(),
                       Options::appstream(),
                       Options::appstreamOutput()});
    parser.process(app);

    // at least one operation should be specified
    if (!parser.isSet(QStringLiteral("hash")) && !parser.isSet(QStringLiteral("g")) && !parser.isSet(QStringLiteral("i")) && !parser.isSet(QStringLiteral("s"))
        && !parser.isSet(QStringLiteral("appstream-metainfo")) && !parser.isSet(QStringLiteral("u")) && !parser.isSet(QStringLiteral("l"))
        && !parser.isSet(QStringLiteral("list-types")) && !parser.isSet(QStringLiteral("r")) && !parser.isSet(QStringLiteral("generate-index"))
        && !parser.isSet(QStringLiteral("remove-index"))) {
        parser.showHelp(0);
    }
    return app.exec();
}
