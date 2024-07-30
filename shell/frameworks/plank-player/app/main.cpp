// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QStandardPaths>
#include <QQmlContext>

#include <KLocalizedContext>
#include <KLocalizedString>

int main(int argc, char *argv[])
{
    QStringList arguments;
    for (int a = 0; a < argc; ++a) {
        arguments << QString::fromLocal8Bit(argv[a]);
    }

    qputenv("QT_VIRTUALKEYBOARD_DESKTOP_DISABLE", QByteArray("0"));
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setOrganizationName(QStringLiteral("PlankPlayer"));
    QCoreApplication::setApplicationName(QStringLiteral("PlankPlayer"));

    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    QQuickStyle::setFallbackStyle(QStringLiteral("Fusion"));
    QGuiApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("plank-player"));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    QUrl homePath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    engine.rootContext()->setContextProperty(QStringLiteral("HomeDirectory"), homePath);

    QString argumentFileUrl;

    if (arguments.count() > 1) {
        QUrl url = QUrl::fromUserInput(arguments.at(1));
        if (url.isValid()) {
            argumentFileUrl = url.toString();
        }
    }

    engine.rootContext()->setContextProperty(QStringLiteral("argumentFileUrl"), argumentFileUrl);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
