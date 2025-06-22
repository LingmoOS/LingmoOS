/*
SPDX-FileCopyrightText: 2017 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "../greeter/pamauthenticator.h"
#include <KUser>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.process(app);
    PamAuthenticator authenticator("kde", KUser().loginName());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("authenticator"), &authenticator);
    engine.load(QUrl::fromLocalFile(QStringLiteral(QML_FILE)));
    return app.exec();
}
