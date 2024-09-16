// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QQuickItem>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include <DLog>

class Object : public QObject {
    Q_OBJECT

public:
    Q_INVOKABLE QByteArray readFile(const QUrl &url) const {
        QFile file(url.isLocalFile() ? url.toLocalFile() : (":" + url.path()));
        if (!file.open(QIODevice::ReadOnly))
            return QByteArray();
        return file.readAll();
    }
    Q_INVOKABLE void replace(QQuickItem *oldItem, QQuickItem *newItem) const {
        newItem->setParentItem(oldItem->parentItem());
        oldItem->setParentItem(nullptr);
        QQmlEngine::setObjectOwnership(oldItem, QQmlEngine::CppOwnership);
        oldItem->deleteLater();
    }

    Q_INVOKABLE QString sceneGraphBackend(QQuickWindow *window) const {
        const QString &backend = window->sceneGraphBackend();
        if (!backend.isEmpty())
            return backend;

        switch (window->rendererInterface()->graphicsApi()) {
        case QSGRendererInterface::Unknown: return "Unknown";
        case QSGRendererInterface::Software: return "Software";
        case QSGRendererInterface::OpenGL: return "OpenGL";
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        case QSGRendererInterface::Direct3D12: return "Direct3D12";
        case QSGRendererInterface::OpenGLRhi: return "OpenGLRhi";
#endif
        case QSGRendererInterface::OpenVG: return "OpenVG";
        case QSGRendererInterface::Direct3D11Rhi: return "Direct3D11Rhi";
        case QSGRendererInterface::VulkanRhi: return "VulkanRhi";
        case QSGRendererInterface::MetalRhi: return "MetalRhi";
        case QSGRendererInterface::NullRhi: return "NullRhi";
        default:
            break;
        }

        return "None";
    }
};

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dtk-exhibition");

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
    Dtk::Core::DLogManager::registerConsoleAppender();
    engine.rootContext()->setContextProperty("examplesFiles",
                                             QDir(":/examples").entryList({"*.qml"}));
    engine.rootContext()->setContextProperty("globalObject", new Object());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}

#include "main.moc"
