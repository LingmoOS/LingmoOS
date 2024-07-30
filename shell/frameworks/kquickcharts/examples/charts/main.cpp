/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addOption({QStringLiteral("page"), QStringLiteral("The page to show."), QStringLiteral("page")});
    parser.addHelpOption();
    parser.process(app);

    QQmlApplicationEngine engine;

    if (parser.isSet(QStringLiteral("page"))) {
        engine.rootContext()->setContextProperty(QStringLiteral("__commandLinePage"), parser.value(QStringLiteral("page")));
    } else {
        engine.rootContext()->setContextProperty(QStringLiteral("__commandLinePage"), nullptr);
    }

    engine.loadFromModule("org.kde.quickcharts.example", "Main");

    return app.exec();
}
