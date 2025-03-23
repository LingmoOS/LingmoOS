// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("Example");
    app.setApplicationVersion("1.0.0");

    QQmlApplicationEngine engine;

    engine.addImportPath(CHAMELEON_PATH);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QQuickStyle::addStylePath(CHAMELEON_PATH);
//    QQuickWindow::setSceneGraphBackend(QSGRendererInterface::Software);
#else
//    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0) && defined(QT_NO_DEBUG)
    QQuickStyle::setStyle(CHAMELEON_PATH"/Chameleon");
#else
    QQuickStyle::setStyle("Chameleon");
#endif

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
