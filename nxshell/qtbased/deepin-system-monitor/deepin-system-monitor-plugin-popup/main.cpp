// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/mainwindow.h"
#include "dde-dock/constants.h"
#include "application.h"
#include "ddlog.h"

//#include "clipboard_adaptor.h"
#include "logger.h"
#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <unistd.h>
#include <QDBusInterface>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

using namespace DDLog;

int main(int argc, char *argv[])
{
#ifdef DTKCORE_CLASS_DConfigFile
    MLogger();   // 日志处理要放在app之前，否则QApplication
            // 内部可能进行了日志打印，导致环境变量设置不生效
#endif

// 为了兼容性
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    Dtk::Core::DLogManager::registerJournalAppender();
#else
    Dtk::Core::DLogManager::registerFileAppender();
#endif
#ifdef QT_DEBUG
    Dtk::Core::DLogManager::registerConsoleAppender();
#endif

    // 前置wayland环境变量
    if (!qgetenv("WAYLAND_DISPLAY").isEmpty()) {
        qputenv("QT_QPA_PLATFORM", "wayland");
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }

    DGuiApplicationHelper::setUseInactiveColorGroup(false);
    DGuiApplicationHelper::setColorCompositingEnabled(true);

    //    DApplication *app = DApplication::globalApplication(argc, argv);

    Application::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    Application ac(argc, argv);
    ac.setAutoActivateWindows(true);

    ac.setOrganizationName("deepin");
    ac.setApplicationName("deepin-system-monitor-plugin-popup");
    ac.setApplicationDisplayName("deepin-system-monitor-plugin-popup");
    ac.setApplicationVersion("1.0");

    if (!DGuiApplicationHelper::setSingleInstance(QString("deepin-system-monitor-plugin-popup"))) {
        qCDebug(app) << "set single instance failed!";
        return -1;
    }

    ac.loadTranslator();

    MainWindow w;
    gApp->setMainWindow(&w);

    QDBusInterface interface("com.deepin.SystemMonitorPluginPopup", "/com/deepin/SystemMonitorPluginPopup",
                             "com.deepin.SystemMonitorPluginPopup",
                             QDBusConnection::sessionBus());
    if (!interface.isValid()) {
        qCDebug(app) << "start loader...";
        w.startLoader();
    }
#ifdef QT_DEBUG
    w.showAni();
#endif
    return ac.exec();
}
