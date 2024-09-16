// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

extern "C" {
#include "camview.h"
}
#include "mainwindow.h"
#include "capplication.h"
#include "cameraconfig.h"
#include "acobjectlist.h"
#include "dbus_adpator.h"

#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#endif

extern "C" {
#include <libimagevisualresult/visualresult.h>
}

#include <DMainWindow>
#include <DWidgetUtil>
#include <DLog>
#include <DApplicationSettings>

#include <QSharedMemory>
#include <QTime>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#ifndef DEEPIN_CAMERA_PRO
#include "config.h"
#else
#define VERSION ""
#endif

DWIDGET_USE_NAMESPACE

//判断是否采用wayland显示服务器
static bool CheckWayland()
{
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return true;
    } else
        return false;
}

static bool CheckFFmpegEnv()
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (QString("libavcodec") + "*"), QDir::NoDotAndDotDot | QDir::Files);
    QString libName = "libavcodec.so"; // set default name for load if not find in LibrariesPath 
    QRegExp re("libavcodec.so.*");   //Sometimes libavcodec.so may not exist, so find it through regular expression.
    for (int i = 0; i < list.count(); i++) {
        if (re.exactMatch(list[i])) {
            libName = list[i];
            break;
        }
    }

    QLibrary libavcodec;   //检查编码器是否存在
    libavcodec.setFileName(libName);
    if (!libavcodec.load()) {
        qWarning() << QString("Not found libavcodec: %1").arg(libavcodec.errorString());
        return false;
    }
    qDebug() << "The libavcodec is loaded successfully.";
    typedef AVCodec *(*p_avcodec_find_encoder)(enum AVCodecID id);
    p_avcodec_find_encoder m_avcodec_find_encoder = nullptr;
    m_avcodec_find_encoder = reinterpret_cast<p_avcodec_find_encoder>(libavcodec.resolve("avcodec_find_encoder"));

    AVCodec *pCodec = nullptr;
    if (m_avcodec_find_encoder)
        pCodec = m_avcodec_find_encoder(AV_CODEC_ID_H264);

    if (pCodec) {
        qDebug() << "Video encoder exists. AVCodecID:" << AV_CODEC_ID_H264;
        DataManager::instance()->setEncExists(true);
    } else {
        qWarning() << "Can not find output video encoder! AVCodecID:" << AV_CODEC_ID_H264;
        DataManager::instance()->setEncExists(false);
    }

    return true;
}

int main(int argc, char *argv[])
{
    // Task 326583 不参与合成器崩溃重连
    unsetenv("QT_WAYLAND_RECONNECT");

    QAccessible::installFactory(accessibleFactory);
    bool bWayland = CheckWayland();
    bool bFFmpegEnv = CheckFFmpegEnv();
    if (argc > 1) {
        if (QString(argv[1]) == "-g") {
            bFFmpegEnv = false;
            qDebug() << "当前为gstreamer测试环境..";
        }
    }
    DataManager::instance()->setEncodeEnv(bFFmpegEnv ? FFmpeg_Env : GStreamer_Env);

    //root login for this application
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    if (bWayland) {

        if (qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") != "TreeLand")
            qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGLES);
        format.setDefaultFormat(format);
        set_wayland_status(1);

        int mp4Encode = -1;
#ifdef DTKCORE_CLASS_DConfigFile
        //需要查询是否对不同机型设置MP4编码缓存特殊处理
        DConfig *dconfig = DConfig::create("org.deepin.camera", "org.deepin.camera.encode");
        if (dconfig && dconfig->isValid() && dconfig->keyList().contains("mp4EncodeMode")) {
            mp4Encode = dconfig->value("mp4EncodeMode").toInt();
            set_pugx_status(mp4Encode);
        }
#endif
        qInfo() << "mp4EncodeMode value is:" << get_pugx_status();
        if (mp4Encode == -1) {
            QStringList options;
            options << QString(QStringLiteral("-c"));
            options << QString(QStringLiteral("dmidecode -t 11 | grep 'String 4' | awk '{print $NF}' && "
                                              "dmidecode -s system-product-name|awk '{print $NF}'"));
            QProcess process;
            process.start(QString(QStringLiteral("bash")), options);
            process.waitForFinished();
            process.waitForReadyRead();
            QByteArray tempArray = process.readAllStandardOutput();
            char *charTemp = tempArray.data();
            QString str_output = QString(QLatin1String(charTemp));
            process.close();

            if (str_output.contains("PGUX", Qt::CaseInsensitive)) {
                mp4Encode = 1;
                qDebug() << "this is PGUX";
            } else {
                mp4Encode = 0;
            }
            qInfo() << "process find mp4EncodeMode value is:" << get_pugx_status();
        }

        set_pugx_status(mp4Encode);
        qInfo() << "last mp4EncodeMode value is:" << get_pugx_status();
    }

    QTime time;
    time.start();
    QString lutDir = LUT_DIR;
    initFilters(lutDir.toStdString().c_str());
    qDebug() << QString("initFilters cost %1 ms").arg(time.elapsed());

    CApplication a(argc, argv);
    //gst_init(&argc, &argv);

    qApp->setObjectName("deepin-camera");
#ifndef __mips__
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    // overwrite DApplication default value
    qApp->setAttribute(Qt::AA_ForceRasterWidgets, false);
#endif

    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
    qApp->setOrganizationName("deepin");
    //设置应用名称
    qApp->setApplicationName("deepin-camera");
    //加载翻译
    qApp->loadTranslator(QList<QLocale>() << QLocale::system());
    //设置程序名称
    qApp->setApplicationDisplayName(QObject::tr("Camera"));
    //设置产品名称
    qApp->setProductName(QObject::tr("Camera"));
    //日志
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qInfo() << "LogFile:" << DLogManager::getlogFilePath();
    //版本
    qApp->setApplicationVersion(DApplication::buildVersion(VERSION));
    QIcon myIcon = QIcon::fromTheme("deepin-camera");
    qApp->setWindowIcon(myIcon);
    qApp->setProductIcon(myIcon);//08月21获悉已添加到系统，故更改为从系统获取

    //应用描述
    qApp->setApplicationDescription(QObject::tr("Camera is an image and video capture utility using your PC camera or webcam."));

    dc::Settings::get().init();

    DApplicationSettings saveTheme;

    if (!qApp->setSingleInstance("deepin-camera")) {
        qDebug() << "another deepin camera instance has started";
        QDBusInterface iface("com.deepin.camera", QDir::separator(), "com.deepin.camera");
        if (iface.isValid()) {
            qDebug() << "deepin-camera raise";
            iface.asyncCall("Raise");
        }

        exit(0);
    }

    CMainWindow w;
    a.setMainWindow(&w);

    Dtk::Widget::moveToCenter(&w);
    w.setWayland(bWayland);
    //判断是否是平板环境
    if (CamApp->isPanelEnvironment())
        w.showMaximized();
    else
        w.setMinimumSize(CMainWindow::minWindowWidth, CMainWindow::minWindowHeight);

    w.show();
    w.loadAfterShow();

    ApplicationAdaptor adaptor(&w);
    QDBusConnection::sessionBus().registerService("com.deepin.camera");
    QDBusConnection::sessionBus().registerObject(QDir::separator(), &w);

    return qApp->exec();
}
