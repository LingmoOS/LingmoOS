// SPDX-FileCopyrightText: 2020 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QGuiApplication>
#include <QQuickView>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQuickView window;
    window.setSource(QUrl(QStringLiteral("qrc:/saverpic.qml")));
    window.showMaximized();

    return app.exec();
}
