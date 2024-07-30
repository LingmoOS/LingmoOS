/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QUrl>

#include <purpose/alternativesmodel.h>
#include <purpose/menu.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Purpose::Menu menu;
    Purpose::AlternativesModel *model = menu.model();

    QJsonObject input;
    if (app.arguments().length() > 1) {
        QMimeDatabase mime;
        QUrl url = QUrl::fromUserInput(app.arguments().constLast());
        input = QJsonObject{{QStringLiteral("urls"), QJsonArray{url.toString()}}, {QStringLiteral("mimeType"), mime.mimeTypeForUrl(url).name()}};
    } else {
        input =
            QJsonObject{{QStringLiteral("urls"), QJsonArray{QStringLiteral("http://kde.org")}}, {QStringLiteral("mimeType"), QStringLiteral("dummy/thing")}};
    }
    qDebug() << "sharing..." << input;

    model->setInputData(input);
    model->setPluginType(QStringLiteral("Export"));
    menu.reload();
    menu.exec();

    QObject::connect(&menu, &Purpose::Menu::finished, &menu, [&app](const QJsonObject &output, int error, const QString &errorMessage) {
        if (error != 0) {
            qDebug() << "job failed with error" << errorMessage;
        }
        qDebug() << "output:" << output;
        app.quit();
    });

    return app.exec();
}
