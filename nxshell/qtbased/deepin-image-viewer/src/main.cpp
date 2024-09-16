// SPDX-FileCopyrightText: 2020 ~ 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/filecontrol.h"
#include "src/cursortool.h"
#include "src/ocr/livetextanalyzer.h"
#include "src/dbus/applicationadpator.h"
#include "src/declarative/mousetrackitem.h"
#include "src/declarative/pathviewrangehandler.h"
#include "src/globalcontrol.h"
#include "src/globalstatus.h"
#include "src/types.h"
#include "src/imagedata/imageinfo.h"
#include "src/imagedata/imagesourcemodel.h"
#include "src/imagedata/imageprovider.h"
#include "src/utils/filetrashhelper.h"
#include "config.h"

#include <DApplication>
#include <DLog>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>
#include <QIcon>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    qputenv("D_POPUP_MODE", "embed");
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    DApplication app(argc, argv);
    app.loadTranslator();
    app.setApplicationLicense("GPLV3");
    app.setApplicationVersion(VERSION);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    // TODO: ? 设置后在 DTK6 下菜单显示空白
    // app.setOrganizationName("deepin");
    app.setApplicationName("deepin-image-viewer");
    app.setApplicationDisplayName(QObject::tr("Image Viewer"));
    app.setProductIcon(QIcon::fromTheme("deepin-image-viewer"));
    app.setApplicationDescription(
        QObject::tr("Image Viewer is an image viewing tool with fashion interface and smooth performance."));
    app.setWindowIcon(QIcon::fromTheme("deepin-image-viewer"));

    // LOG
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qInfo() << QString("%1 start, PID: %2, Version: %3")
                   .arg(app.applicationName())
                   .arg(app.applicationPid())
                   .arg(app.applicationVersion());
    qDebug() << "LogFile:" << DLogManager::getlogFilePath();

    QQmlApplicationEngine engine;

    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    // @uri org.deepin.image.viewer
    const QString uri("org.deepin.image.viewer");
    qmlRegisterType<ImageInfo>(uri.toUtf8().data(), 1, 0, "ImageInfo");
    qmlRegisterUncreatableType<ImageSourceModel>(uri.toUtf8().data(), 1, 0, "ImageSourceModel", "Use for global data");
    qmlRegisterUncreatableType<PathViewProxyModel>(uri.toUtf8().data(), 1, 0, "PathViewProxyModel", "Use for view data");
    qmlRegisterType<MouseTrackItem>(uri.toUtf8().data(), 1, 0, "MouseTrackItem");
    qmlRegisterType<PathViewRangeHandler>(uri.toUtf8().data(), 1, 0, "PathViewRangeHandler");

    qmlRegisterUncreatableType<Types>(uri.toUtf8().data(), 1, 0, "Types", "Types only use for define");
    // 文件回收站处理
    qmlRegisterType<FileTrashHelper>(uri.toUtf8().data(), 1, 0, "FileTrashHelper");

    // QML全局单例
    GlobalControl control;
    qmlRegisterSingletonInstance<GlobalControl>(uri.toUtf8().data(), 1, 0, "GControl", &control);
    GlobalStatus status;
    qmlRegisterSingletonInstance<GlobalStatus>(uri.toUtf8().data(), 1, 0, "GStatus", &status);
    FileControl fileControl;
    qmlRegisterSingletonInstance<FileControl>(uri.toUtf8().data(), 1, 0, "FileControl", &fileControl);
    // 光标位置查询工具
    CursorTool cursorTool;
    qmlRegisterSingletonInstance<CursorTool>(uri.toUtf8().data(), 1, 0, "CursorTool", &cursorTool);

    // 解析命令行参数
    QString cliParam = fileControl.parseCommandlineGetPath();

    // 后端缩略图加载，由 QMLEngine 管理生命周期
    // 部分平台支持线程数较低时，使用同步加载
    ProviderCache *providerCache = nullptr;
    if (!GlobalControl::enableMultiThread()) {
        ImageProvider *imageProvider = new ImageProvider;
        engine.addImageProvider(QLatin1String("ImageLoad"), imageProvider);

        providerCache = static_cast<ProviderCache *>(imageProvider);
    } else {
        AsyncImageProvider *asyncImageProvider = new AsyncImageProvider;
        engine.addImageProvider(QLatin1String("ImageLoad"), asyncImageProvider);

        providerCache = static_cast<ProviderCache *>(asyncImageProvider);

        if (!cliParam.isEmpty()) {
            asyncImageProvider->preloadImage(cliParam);
        }
    }

    ThumbnailProvider *multiImageLoad = new ThumbnailProvider;
    engine.addImageProvider(QLatin1String("ThumbnailLoad"), multiImageLoad);

    // 关联各组件
    // 图片旋转时更新图像缓存
    QObject::connect(&control, &GlobalControl::requestRotateCacheImage, [&]() {
        providerCache->rotateImageCached(control.currentRotation(), control.currentSource().toLocalFile());
    });

    status.setEnableNavigation(fileControl.isEnableNavigation());
    QObject::connect(
        &status, &GlobalStatus::enableNavigationChanged, [&]() { fileControl.setEnableNavigation(status.enableNavigation()); });
    QObject::connect(&fileControl, &FileControl::imageRenamed, &control, &GlobalControl::renameImage);
    // 文件变更时清理缓存
    QObject::connect(&fileControl, &FileControl::imageFileChanged, [&](const QString &fileName) {
        providerCache->removeImageCache(fileName);
    });

    // OCR分析工具
    auto liveTextAnalyzer = new LiveTextAnalyzer;
    engine.rootContext()->setContextProperty("liveTextAnalyzer", liveTextAnalyzer);
    engine.addImageProvider(QLatin1String("liveTextAnalyzer"), liveTextAnalyzer);

    // 判断命令行数据，在 QML 前优先加载
    if (!cliParam.isEmpty()) {
        QStringList filePaths = fileControl.getDirImagePath(cliParam);
        if (!filePaths.isEmpty()) {
            control.setImageFiles(filePaths, cliParam);
            fileControl.resetImageFiles(filePaths);

            status.setStackPage(Types::ImageViewPage);
        }
    }

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    // 设置DBus接口
    ApplicationAdaptor adaptor(&fileControl);
    QDBusConnection::sessionBus().registerService("com.deepin.imageViewer");
    QDBusConnection::sessionBus().registerObject("/", &fileControl);

    return app.exec();
}
