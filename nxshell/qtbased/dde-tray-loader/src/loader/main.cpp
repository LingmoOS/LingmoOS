// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "setproctitle.h"
#include "widgetplugin.h"
#include "pluginsiteminterface_v2.h"

#include <DDBusSender>
#include <DApplication>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <QWidget>
#include <QPluginLoader>
#include <QStringLiteral>

#include <cstdlib>
#include <DGuiApplicationHelper>
#include <DStandardPaths>
#include <DPathBuf>
#include <LogManager.h>
#include <signal.h>

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

static QString pluginDisplayName;

[[noreturn]] void sig_crash(int signum)
{
    DDBusSender()
        .service("org.deepin.dde.Notification1")
        .path("/org/deepin/dde/Notification1")
        .interface("org.deepin.dde.Notification1")
        .method(QString("Notify"))
        .arg(QString("dde-control-center"))
        .arg(static_cast<uint>(0))
        .arg(QString("preferences-system"))
        .arg(QString("Tray Plugin Crashed!"))
        .arg(QString("%1 plugin is crashed").arg(pluginDisplayName))
        .arg(QStringList())
        .arg(QVariantMap())
        .arg(2000)
        .call();

#ifndef QT_DEBUG
    // 重新触发信号，产生coredump
    signal(signum, SIG_DFL);
    raise(signum);
#endif
}

int main(int argc, char *argv[], char *envp[])
{
#ifndef QT_DEBUG
    // 设置信号处理函数
    struct sigaction sa;
    sa.sa_handler = sig_crash;
    sigemptyset(&sa.sa_mask);
    // 在处理完信号后恢复默认信号处理
    sa.sa_flags = SA_RESETHAND;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
#endif

    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    Dtk::Widget::DApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    init_setproctitle(argv, envp);
    qputenv("DSG_APP_ID", "dde-dock");
    qputenv("WAYLAND_DISPLAY", "dockplugin");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "plugin-shell");
    qputenv("QT_WAYLAND_RECONNECT", "1");

    Dtk::Widget::DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setQuitOnLastWindowClosed(false);

    // 创建翻译器
    QTranslator translator;
    if (translator.load(QString("/usr/share/trayplugin-loader/translations/trayplugin-loader_%1").arg(QLocale().name()))) {
        app.installTranslator(&translator);
    }

    QList<QString> translateDirs;
    auto dataDirs = DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &path : dataDirs) {
        DPathBuf DPathBuf(path);
        translateDirs << (DPathBuf / "dde-dock/translations").toString();
    }
    DGuiApplicationHelper::loadTranslator("dde-dock", translateDirs, QList<QLocale>() << QLocale::system());

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption pluginOption("p", "special plugin path.", "path to plugin", QString());
    parser.addOption(pluginOption);
    parser.process(app);

    if (!parser.isSet(pluginOption)) {
        parser.showHelp(0);
    }

    QString plugin = parser.value(pluginOption);

 #ifdef QT_DEBUG
    const QDir shellDir(QString("%1/../../plugins/").arg(QCoreApplication::applicationDirPath()));
    if (shellDir.exists()) {
        QCoreApplication::addLibraryPath(shellDir.absolutePath());
    }
#endif
    QPluginLoader* pluginLoader = new QPluginLoader(plugin, &app);
    const QJsonObject &meta = pluginLoader->metaData().value("MetaData").toObject();
    const QString &pluginApi = meta.value("api").toString();

    if (!pluginLoader->load()) {
        qDebug() << pluginLoader->errorString();
        return 0;
    }

    PluginsItemInterface *interface = qobject_cast<PluginsItemInterfaceV2 *>(pluginLoader->instance());
    if (!interface) {
        interface = qobject_cast<PluginsItemInterface*>(pluginLoader->instance());
    }
    if (!interface) {
        qWarning() << "get interface failed!" << pluginLoader->instance() << qobject_cast<PluginsItemInterfaceV2*>(pluginLoader->instance());;
        return -1;
    }
    pluginDisplayName = interface->pluginDisplayName();
    dock::WidgetPlugin dockPlugin(interface, pluginLoader->instance());

    DLogManager::setLogFormat("%{time}{yy-MM-ddTHH:mm:ss.zzz} [%{type}] [%{category}] <%{function}> %{message}");

    DLogManager::registerJournalAppender();
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    app.setApplicationName(interface->pluginName());
    app.setApplicationDisplayName(interface->pluginDisplayName());
    setproctitle((QStringLiteral("tray plugin: ") + interface->pluginName()).toStdString().c_str());
    qputenv("QT_SCALE_FACTOR", "");
    return app.exec();
}
