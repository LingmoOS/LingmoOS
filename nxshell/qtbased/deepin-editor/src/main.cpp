// SPDX-FileCopyrightText: 2011-2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "utils.h"
#include "window.h"
#include "urlinfo.h"
#include "editorapplication.h"
#include "performancemonitor.h"
#include "eventlogutils.h"
#include "common/utils.h"

#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DLog>
#include <DApplicationSettings>
#include <DSettingsOption>

#include <QApplication>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QScreen>
#include <QDebug>

#include <iostream>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DCORE_USE_NAMESPACE
    PerformanceMonitor::initializeAppStart();
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    if (Utils::isWayland() && !Utils::isTreeland()) {
        qputenv("QT_WAYLAND_SHELL_INTEGRATION", "kwayland-shell");
    }
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    // QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    EditorApplication app(argc, argv);
    // save theme
    DApplicationSettings savetheme;

    // 需在App构造后初始化日志设置
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // Parser input arguments.
    QCommandLineParser parser;
    const QCommandLineOption newWindowOption("w", "Open file in new window");
    const QCommandLineOption helpOption = parser.addHelpOption();
    parser.addOption(newWindowOption);
    parser.process(app);

    qInfo() << qPrintable(QString("App start, pid: %1, version: %2").arg(app.applicationPid()).arg(app.applicationVersion()));

    QStringList urls;
    QStringList arguments = parser.positionalArguments();

    for (const QString &path : arguments) {
        UrlInfo info(path);
        urls << info.url.toLocalFile();

        QFileInfo fileInfo(path);
        qInfo() << qPrintable(QString("Open file, isFile: %1, suffix: %2, size: %3, permssion: %4").arg(fileInfo.isFile())
                              .arg(fileInfo.suffix()).arg(fileInfo.size()).arg(fileInfo.permissions()));
    }

    bool hasWindowFlag = parser.isSet(newWindowOption);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    // Start editor process if not found any editor use DBus.
    if (dbus.registerService("com.deepin.Editor")) {
#ifdef DTKWIDGET_CLASS_DSizeMode
        // 不同模式下的基础字体像素大小不同，系统级别为 T6 的字体大小, 默认是 14px ；在紧凑模式下 T6 为 12px
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, []() {
            const int genericPixelSize = 14;
            const int genericPixelSizeCompact = 12;
            DFontSizeManager::instance()->setFontGenericPixelSize(
                DGuiApplicationHelper::isCompactMode() ? genericPixelSizeCompact : genericPixelSize);
        });
#endif

        StartManager *startManager = StartManager::instance();
        //埋点记录启动数据
        QJsonObject objStartEvent{
            {"tid", Eventlogutils::StartUp},
            {"version", VERSION},
            {"mode", 1},
        };
        Eventlogutils::GetInstance()->writeLogs(objStartEvent);

        bool save_tab_before_close =
            Settings::instance()->settings->option("advance.startup.save_tab_before_close")->value().toBool();
        if (!save_tab_before_close && urls.isEmpty()) {
            auto window = startManager->createWindow(true);
            window->addBlankTab();
        } else {
            if (hasWindowFlag) {
                startManager->openFilesInWindow(urls);
            } else {
                startManager->openFilesInTab(urls);
            }
        }
#if _ZPD_
        // 解析ZPD定制需求提供的库libzpdcallback.so
        Utils::loadCustomDLL();
#endif
        dbus.registerObject("/com/deepin/Editor", startManager, QDBusConnection::ExportScriptableSlots);

        PerformanceMonitor::initializAppFinish();
        return app.exec();
    }
    // Just send dbus message to exist editor process.
    else {
        QDBusInterface notification(
            "com.deepin.Editor", "/com/deepin/Editor", "com.deepin.Editor", QDBusConnection::sessionBus());

        QList<QVariant> args;
        args << urls;

        if (hasWindowFlag) {
            notification.callWithArgumentList(QDBus::AutoDetect, "openFilesInWindow", args);
        } else {
            notification.callWithArgumentList(QDBus::AutoDetect, "openFilesInTab", args);
        }
    }

    return 0;
}
