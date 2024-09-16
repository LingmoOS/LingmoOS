// SPDX-FileCopyrightText: 2020 ~ 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "src/filecontrol.h"
#include "src/thumbnailload.h"
#include "src/cursortool.h"
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

#include "src/albumControl.h"
#include "src/imageengine/imagedataservice.h"
#include "thumbnailview/itemviewadapter.h"
#include "thumbnailview/positioner.h"
#include "thumbnailview/rubberband.h"
#include "thumbnailview/mouseeventlistener.h"
#include "thumbnailview/eventgenerator.h"
#include "thumbnailview/roles.h"
#include "thumbnailview/imagedatamodel.h"
#include "thumbnailview/thumbnailmodel.h"
#include "thumbnailview/qimageitem.h"

#include <DGuiApplicationHelper>
#include <DApplication>

#include <DLog>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScopedPointer>
#include <QQmlContext>
#include <QIcon>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

int main(int argc, char *argv[])
{
    //qputenv("QML_DISABLE_DISK_CACHE", "1");
    qputenv("D_POPUP_MODE", "embed");
    if (qEnvironmentVariableIsEmpty("XDG_CURRENT_DESKTOP")) {
        qputenv("XDG_CURRENT_DESKTOP", "Deepin");
    }

    DApplication app(argc, argv);
    app.loadTranslator();
    app.setApplicationLicense("GPLV3");
    app.setApplicationVersion(DApplication::buildVersion(VERSION));
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    // TODO: ? 设置后在 DTK6 下菜单显示空白
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-album");
    app.setApplicationDisplayName(QObject::tr("Album"));
    app.setProductIcon(QIcon::fromTheme("deepin-album"));
    app.setApplicationDescription(
        QObject::tr("Main", "Album is a fashion manager for viewing and organizing photos and videos."));
    app.setWindowIcon(QIcon::fromTheme("deepin-album"));

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QQmlApplicationEngine engine;

    if (!DGuiApplicationHelper::instance()->setSingleInstance(app.applicationName(), DGuiApplicationHelper::UserScope)) {
        exit(0);
    }

    // 配置文件加载
    LibConfigSetter::instance()->loadConfig(imageViewerSpace::ImgViewerTypeAlbum);

    // 请在此处注册需要导入到QML中的C++类型
    // 例如： engine.rootContext()->setContextProperty("Utils", new Utils);
    // @uri org.deepin.album
    const QString uri("org.deepin.image.viewer");
    qmlRegisterType<ImageInfo>(uri.toUtf8().data(), 1, 0, "ImageInfo");
    qmlRegisterUncreatableType<ImageSourceModel>(uri.toUtf8().data(), 1, 0, "ImageSourceModel", "Use for global data");
    qmlRegisterUncreatableType<PathViewProxyModel>(uri.toUtf8().data(), 1, 0, "PathViewProxyModel", "Use for view data");
    qmlRegisterType<MouseTrackItem>(uri.toUtf8().data(), 1, 0, "MouseTrackItem");
    qmlRegisterType<PathViewRangeHandler>(uri.toUtf8().data(), 1, 0, "PathViewRangeHandler");
    // 文件回收站处理
    qmlRegisterType<FileTrashHelper>(uri.toUtf8().data(), 1, 0, "FileTrashHelper");

    // QML全局单例
    GlobalControl control;
    engine.rootContext()->setContextProperty("GControl", &control);
    GlobalStatus status;
    engine.rootContext()->setContextProperty("GStatus", &status);
    FileControl fileControl;
    engine.rootContext()->setContextProperty("FileControl", &fileControl);
    status.setFileControl(&fileControl);
    // 光标位置查询工具
    CursorTool cursorTool;
    engine.rootContext()->setContextProperty("CursorTool", &cursorTool);

    // 解析命令行参数
    QString cliParam = /*fileControl.parseCommandlineGetPath()*/"";

    // 看图后端缩略图加载，由 QMLEngine 管理生命周期
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

    // 判断命令行数据，在 QML 前优先加载
    if (!cliParam.isEmpty()) {
        QStringList filePaths = fileControl.getDirImagePath(cliParam);
        if (!filePaths.isEmpty()) {
            control.setImageFiles(filePaths, cliParam);
            fileControl.resetImageFiles(filePaths);

            status.setStackPage(Types::ImageViewPage);
        }
    }

    QImageItem::initDamage();
    //设置为相册模式
    fileControl.setViewerType(imageViewerSpace::ImgViewerTypeAlbum);

    //禁止多开
    if (!fileControl.isCheckOnly()) {
        return 0;
    }

    //新增相册控制模块
    engine.rootContext()->setContextProperty("albumControl", AlbumControl::instance());

    //相册后端缩略图加载
    LoadImage *load = new LoadImage();
    engine.addImageProvider(QLatin1String("collectionPublisher"), load->m_collectionPublisher);
    engine.addImageProvider(QLatin1String("asynImageProviderAlbum"), load->m_asynImageProvider);
    engine.rootContext()->setContextProperty("asynImageProviderAlbum", load->m_asynImageProvider);
    engine.rootContext()->setContextProperty("imageDataService", ImageDataService::instance());

    char uriAlbum[] = "org.deepin.album";
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<QAbstractItemModel>();
#else
    qmlRegisterAnonymousType<QAbstractItemModel>(uriAlbum, 0);
#endif
    qmlRegisterType<ImageDataModel>(uriAlbum, 1, 0, "ImageDataModel");
    qmlRegisterType<ThumbnailModel>(uriAlbum, 1, 0, "ThumbnailModel");
    qmlRegisterType<ItemViewAdapter>(uriAlbum, 1, 0, "ItemViewAdapter");
    qmlRegisterType<Positioner>(uriAlbum, 1, 0, "Positioner");
    qmlRegisterType<RubberBand>(uriAlbum, 1, 0, "RubberBand");
    qmlRegisterType<MouseEventListener>(uriAlbum, 1, 0, "MouseEventListener");
    qmlRegisterType<EventGenerator>(uriAlbum, 1, 0, "EventGenerator");
    qmlRegisterUncreatableType<Types>(uriAlbum, 1, 0, "Types", "Cannot instantiate the Types class");
    qmlRegisterUncreatableType<Roles>(uriAlbum, 1, 0, "Roles", "Cannot instantiate the Roles class");
    qmlRegisterType<QImageItem>(uriAlbum, 1, 0, "QImageItem");

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    // 设置DBus接口
    ApplicationAdaptor adaptor(&fileControl);
    QDBusConnection::sessionBus().registerService("com.deepin.album");
    QDBusConnection::sessionBus().registerObject("/", &fileControl);

    return app.exec();
}
