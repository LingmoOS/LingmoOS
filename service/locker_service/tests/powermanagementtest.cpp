/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "../greeter/powermanagement.h"
#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QQuickView view;
    view.rootContext()->setContextProperty(QStringLiteral("powerManagement"), PowerManagement::instance());
    view.setSource(QUrl::fromLocalFile(QStringLiteral(QML_PATH)));

    view.show();

    return app.exec();
}
