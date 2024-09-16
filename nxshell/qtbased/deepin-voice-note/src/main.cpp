// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "vnoteapplication.h"
#include "views/vnotemainwindow.h"
#include "globaldef.h"
#include "common/performancemonitor.h"
#include "common/utils.h"

#include <QDir>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QStandardPaths>
#include <QLibraryInfo>

#include <DApplication>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DMainWindow>
#include <DLog>
#include <DWidgetUtil>
#include <DPlatformWindowHandle>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // Task 326583 不参与合成器崩溃重连
    unsetenv("QT_WAYLAND_RECONNECT");

    PerformanceMonitor::initializeAppStart();
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    //开启QWebEngineView的调试模式
    if (2 == argc && "--debug" == QString(argv[1])) {
        qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "7777");
    }
    //龙芯机器配置,使得DApplication能正确加载QTWEBENGINE
    qputenv("DTK_FORCE_RASTER_WIDGETS", "FALSE");

#ifdef __sw_64__
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#endif
    //龙芯平台删除字体库，解决QWebEngine因字体库字体太多，造成启动失败的问题
    if (Utils::isLoongsonPlatform()) {
        QString strHomePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        strHomePath.append("/.cache/fontconfig");
        QDir dir(strHomePath);
        dir.removeRecursively();
    }

    VNoteApplication app(argc, argv);
    if (!DPlatformWindowHandle::pluginVersion().isEmpty()) {
        app.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    }

    //设置opengl类型
    QOpenGLContext ctx;
    QSurfaceFormat fmt;
    fmt.setRenderableType(QSurfaceFormat::OpenGL);
    ctx.setFormat(fmt);
    if (!ctx.create() || !ctx.isValid() || ctx.format().renderableType() != QSurfaceFormat::OpenGL) {
        fmt.setRenderableType(QSurfaceFormat::OpenGLES);
        fmt.setDefaultFormat(fmt);
        qInfo() << "set openGLES";
    }

    QString strWebEngineArgs = "--single-process";
    //wayland 模式禁用gpu
    if (Utils::isWayland()) {
        strWebEngineArgs += " --disable-gpu";
    }
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", strWebEngineArgs.toLocal8Bit());

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName(DEEPIN_VOICE_NOTE);
    app.setApplicationVersion(VERSION);
    app.setProductIcon(QIcon::fromTheme(DEEPIN_VOICE_NOTE));
    app.setProductName(DApplication::translate("AppMain", "Voice Notes"));
    app.setApplicationDisplayName(DApplication::translate("AppMain", "Voice Notes"));
    app.setApplicationDescription(DApplication::translate("AppMain",
                                                          "Voice Notes is a lightweight memo tool to make text notes and voice recordings."));

    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");

    DGuiApplicationHelper::instance()->setSingleInstanceInterval(-1);
    if (!DGuiApplicationHelper::instance()->setSingleInstance(
                app.applicationName(),
                DGuiApplicationHelper::UserScope)) {
        return 0;
    }

    DApplicationSettings settings;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    app.activateWindow();

    return app.exec();
}
