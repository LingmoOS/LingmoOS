/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>

#include <qtestcase.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView view;
    QQmlContext *context = view.rootContext();

    QImage image;
    if (app.devicePixelRatio() < 2) {
        image = QImage(QFINDTESTDATA("testimage.png"));
    } else {
        image = QImage(QFINDTESTDATA("testimage@2x.png"));
        image.setDevicePixelRatio(2);
    }

    context->setContextProperty(QStringLiteral("testImage"), image);

    view.setSource(QUrl::fromLocalFile(QFINDTESTDATA("qimageitemtest.qml")));
    view.show();

    return app.exec();
}
