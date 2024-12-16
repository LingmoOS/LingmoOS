// Copyright (C) 2023 wenhao Peng <pengwenhao@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.addImportPath(":/qt/qml");
    engine.load(QUrl(u"qrc:/qt/qml/Wallpaper/main.qml"_qs));

    return app.exec();
}
