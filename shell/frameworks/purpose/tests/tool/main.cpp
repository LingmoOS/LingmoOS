/*
 SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

 SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "qqml.h"
#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <purpose/alternativesmodel.h>
#include <purpose/job.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral(TRANSLATION_DOMAIN));
    KAboutData data(QStringLiteral("sharetool"), i18n("Share Tool"), QStringLiteral("1.0"), i18n("Share random information"), KAboutLicense::GPL);
    data.addAuthor(QStringLiteral("Aleix Pol i Gonzalez"), i18n("Implementation"), QStringLiteral("aleixpol@kde.org"));
    KAboutData::setApplicationData(data);

    QJsonObject inputData;
    QStringList files;
    {
        QCommandLineParser parser;
        parser.addPositionalArgument(QStringLiteral("files"), i18n("Files to share"), i18n("[files...]"));
        parser.addOption(QCommandLineOption(QStringLiteral("data"), i18n("Data tuple to initialize the process with"), QStringLiteral("json")));

        data.setupCommandLine(&parser);
        parser.process(app);
        data.processCommandLine(&parser);

        if (parser.isSet(QStringLiteral("data"))) {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(parser.value(QStringLiteral("data")).toLatin1(), &error);
            if (error.error) {
                qCritical() << qPrintable(i18n("Error in the data argument formatting: %1 at %2", error.errorString(), error.offset));
                parser.showHelp(2);
            } else if (!doc.isObject()) {
                qCritical() << qPrintable(i18n("Error in the data argument type, it should be a json object."));
                parser.showHelp(3);
            }
            inputData = doc.object();
        }

        files = parser.positionalArguments();
        if (files.isEmpty() && !inputData.contains(QStringLiteral("urls"))) {
            qCritical() << qPrintable(i18n("Must specify some files to share"));
            parser.showHelp(1);
        }
    }

    QMimeType common;
    QJsonArray urls;
    if (!files.isEmpty() && (!inputData.contains(QStringLiteral("urls")) || !inputData.contains(QStringLiteral("mimeType")))) {
        QMimeDatabase db;
        for (const QString &file : std::as_const(files)) {
            const QUrl url = QUrl::fromUserInput(file, QString(), QUrl::AssumeLocalFile);
            QMimeType type = db.mimeTypeForUrl(url);
            if (!common.isValid()) {
                common = type;
            } else if (common.inherits(type.name())) {
                common = type;
            } else if (type.inherits(common.name())) {
                ;
            } else {
                common = db.mimeTypeForName(QStringLiteral("application/octet-stream"));
            }
            urls += url.toString();
        }
        inputData.insert(QStringLiteral("urls"), urls);
        inputData.insert(QStringLiteral("mimeType"), common.name());
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.setInitialProperties({
        {QStringLiteral("inputData"), inputData},
    });
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
