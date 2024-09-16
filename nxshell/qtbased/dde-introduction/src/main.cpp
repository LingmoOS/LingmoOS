// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <DPlatformWindowHandle>
#include <DWidget>
#include <DWidgetUtil>
#include <DWindowManagerHelper>
#include <QDBusInterface>
#include <QDebug>
#include "mainwindow.h"
//#include "normalwindow.h"
#include "environments.h"

#ifndef DISABLE_VIDEO
#include <compositing_manager.h>
#endif

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    using namespace Dtk::Core;

    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    /* 平台支持播放时才对播放视频做相关设置 */
    #ifndef DISABLE_VIDEO
    /* 规避兼容103x环境下编译构建失败的问题，因为103x环境无dmr::utils::first_check_wayland_env()接口 */
    #if (DTK_VERSION > DTK_VERSION_CHECK(5, 4, 1, 0))
    /* 适配wayland系统环境 */
    if (dmr::utils::first_check_wayland_env()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
        setenv("PULSE_PROP_media.role", "video", 1);
    }
    #endif
    #endif

    QDateTime current = QDateTime::currentDateTime();
    qDebug() << "LOG_FLAG"
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to initaalize app";
    qint64 initializeAppStartMs = current.toMSecsSinceEpoch();

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DApplication *a = nullptr;

    #if (DTK_VERSION < DTK_VERSION_CHECK(5, 4, 0, 0))
        a = new DApplication(argc, argv);
    #else
        a = DApplication::globalApplication(argc, argv);
    #endif

    a->setAttribute(Qt::AA_UseHighDpiPixmaps);
    a->setApplicationName("dde-introduction");
    a->setAttribute(Qt::AA_ForceRasterWidgets, false);

    // dapplication default setting is true

    if (!a->setSingleInstance(a->applicationName(), DApplication::UserScope)) {
        qWarning() << QString("There is a %1 running!!").arg(a->applicationName());
        return -1;
    }

    a->setAutoActivateWindows(true);
    a->setOrganizationName("deepin");
    a->setApplicationVersion(VERSION);
    //因为快速启动turbo原因，loadTranslator函数需要放在所有QObject::tr之前，setApplicationName之后
    a->loadTranslator();
    a->setApplicationDisplayName(QObject::tr("Welcome"));

    /* 必须在setOrganizationName("deepin")之后设置才生效 */
    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("com.deepin.introduction");

    QCommandLineParser cmdParser;
    cmdParser.setApplicationDescription("dde-introduction");
    cmdParser.addHelpOption();
    cmdParser.addVersionOption();
    cmdParser.process(*a);

    //监听当前应用主题切换事件
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
                     [](DGuiApplicationHelper::ColorType type) {
                         DGuiApplicationHelper::instance()->setPaletteType(type);
                    });

    static const QDate buildDate = QLocale(QLocale::English).toDate(QString(__DATE__).replace("  ", " 0"), "MMM dd yyyy");
    QString t_date = buildDate.toString("MMdd");
    // Version Time
    a->setApplicationVersion(DApplication::buildVersion(t_date));

    setlocale(LC_NUMERIC, "C");

    MainWindow w;
    moveToCenter(&w);
    w.show();

    dbus.registerObject("/com/deepin/introduction", &w, QDBusConnection::ExportScriptableSlots);

    current = QDateTime::currentDateTime();
    qDebug() << "LOG_FLAG"
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to initialize app";

    qint64 inittalizeApoFinishMs = current.toMSecsSinceEpoch();
    qint64 time = inittalizeApoFinishMs - initializeAppStartMs;
    qInfo() << QString("[GRABPOINT] POINT-01  time=%1ms").arg(time);


    return a->exec();
}
