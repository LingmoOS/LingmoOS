/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <kiconloader.h>
#include <kiconthemes_version.h>

#include <QCommandLineParser>
#include <QGuiApplication>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kiconfinder"));
    app.setApplicationVersion(QStringLiteral(KICONTHEMES_VERSION_STRING));
    QCommandLineParser parser;
    parser.setApplicationDescription(app.translate("main", "Finds an icon based on its name"));
    parser.addPositionalArgument(QStringLiteral("iconname"), app.translate("main", "The icon name to look for"));
    parser.addHelpOption();

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
    }

    for (const QString &iconName : parser.positionalArguments()) {
        const QString icon = KIconLoader::global()->iconPath(iconName, KIconLoader::Desktop /*TODO configurable*/, true);
        if (!icon.isEmpty()) {
            printf("%s\n", icon.toLocal8Bit().constData());
        } else {
            return 1; // error
        }
    }

    return 0;
}
