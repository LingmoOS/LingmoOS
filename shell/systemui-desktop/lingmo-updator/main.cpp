/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
// #include <QtWebEngine>
#include <QDBusConnection>
#include <QQmlContext>
#include <QTranslator>
#include <QLocale>
#include <QFile>

#include "updatorhelper.h"
#include "upgradeablemodel.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // QtWebEngine::initialize();
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif        

    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QGuiApplication app(argc, argv);

    const char *uri = "Lingmo.Updator";
    qmlRegisterType<UpdatorHelper>(uri, 1, 0, "Updator");
    qmlRegisterType<UpgradeableModel>(uri, 1, 0, "UpgradeableModel");

    if (!QDBusConnection::sessionBus().registerService("com.lingmo.UpdatorGui")) {
        return 0;
    }

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-updator/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QGuiApplication::instance());
        if (translator->load(qmFilePath)) {
            QGuiApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.rootContext()->setContextProperty("upgradeableModel", UpgradeableModel::self());
    engine.load(url);

    return app.exec();
}
