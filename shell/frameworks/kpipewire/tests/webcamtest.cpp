/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    {
        QCommandLineParser parser;
        parser.addHelpOption();
        parser.process(app);

        auto engine = new QQmlApplicationEngine(&app);
        QObject::connect(engine, &QQmlEngine::quit, qGuiApp, &QCoreApplication::quit);
        engine->load(QUrl(QStringLiteral("qrc:/webcamtest.qml")));
    }

    return app.exec();
}
