/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "PlasmaRecordMe.h"
#include <QCommandLineParser>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    {
        QCommandLineParser parser;
        QCommandLineOption duration(QStringLiteral("duration"), QStringLiteral("seconds length of the video"), QStringLiteral("duration"));
        const QMap<QString, Screencasting::CursorMode> cursorOptions = {
            {QStringLiteral("hidden"), Screencasting::CursorMode::Hidden},
            {QStringLiteral("embedded"), Screencasting::CursorMode::Embedded},
            {QStringLiteral("metadata"), Screencasting::CursorMode::Metadata},
        };

        QCommandLineOption cursor(QStringLiteral("cursor"),
                                  QStringList(cursorOptions.keys()).join(QStringLiteral(", ")),
                                  QStringLiteral("mode"),
                                  QStringLiteral("metadata"));

        QCommandLineOption selection(QStringLiteral("selection"), QStringLiteral("Select a region to show"));
        parser.addPositionalArgument(QStringLiteral("source"), QStringLiteral("name of the output/window that you want to see"));
        parser.addOption(duration);
        parser.addOption(cursor);
        parser.addOption(selection);
        parser.addHelpOption();
        parser.process(app);

        PlasmaRecordMe *me = new PlasmaRecordMe(cursorOptions[parser.value(cursor).toLower()], parser.positionalArguments(), parser.isSet(selection), &app);
        if (parser.isSet(duration)) {
            me->setDuration(parser.value(duration).toInt() * 1000);
        }
    }

    return app.exec();
}
