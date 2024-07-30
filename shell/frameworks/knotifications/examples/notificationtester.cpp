/*
   SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QDebug>
#include <QGuiApplication>
#include <QObject>
#include <QQmlApplicationEngine>

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    engine.load(QStringLiteral("qrc:/notificationtester.qml"));

    return app.exec();
}
