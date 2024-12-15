// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "setproctitle.h"
#include "pluginsiteminterface_v2.h"
#include "pluginmanager.h"

#include <DDBusSender>
#include <DApplication>

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <QStringLiteral>

#include <DGuiApplicationHelper>
#include <DStandardPaths>
#include <DPathBuf>
#include <LogManager.h>
#include <qglobal.h>
#ifndef QT_DEBUG
#include <signal.h>
#endif

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

static QString pluginDisplayName;

void sig_crash(int signum)
{
    DDBusSender()
        .service("org.lingmo.ocean.Notification1")
        .path("/org/lingmo/ocean/Notification1")
        .interface("org.lingmo.ocean.Notification1")
        .method(QString("Notify"))
        .arg(QString("ocean-control-center"))
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

class LoaderApplication : public Dtk::Widget::DApplication
{
public:
    LoaderApplication(int &argc, char **argv) : Dtk::Widget::DApplication(argc, argv) {}

    bool notify(QObject *obj, QEvent *event) Q_DECL_OVERRIDE {
        // fix plugin menu cannot show
        if (event->type() == QEvent::OrientationChange) {
            return true;
        }

        return Dtk::Widget::DApplication::notify(obj, event);
    }
};

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
    init_setproctitle(argv, envp);
    qputenv("DSG_APP_ID", "org.lingmo.ocean-tray-loader");
    qputenv("WAYLAND_DISPLAY", "dockplugin");
    qputenv("QT_WAYLAND_SHELL_INTEGRATION", "plugin-shell");
    qputenv("QT_WAYLAND_RECONNECT", "1");

    LoaderApplication app(argc, argv);
    app.setOrganizationName("lingmo");
    app.setApplicationName("org.lingmo.ocean-tray-loader");
    app.setQuitOnLastWindowClosed(false);

    QList<QString> translateDirs;
    auto dataDirs = DStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &path : dataDirs) {
        DPathBuf DPathBuf(path);
        translateDirs << (DPathBuf / "ocean-dock/translations").toString();
        translateDirs << (DPathBuf / "trayplugin-loader/translations").toString();
    }
    DGuiApplicationHelper::loadTranslator("ocean-dock", translateDirs, QList<QLocale>() << QLocale::system());
    DGuiApplicationHelper::loadTranslator("trayplugin-loader", translateDirs, QList<QLocale>() << QLocale::system());

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption pluginPathsOption(
        "p",
        "plugin path (single path or multiple paths separated by ';').",
        "plugin path(s)"
    );
    QCommandLineOption pluginGroupNameOption(
        "g",
        "Group name for the specified plugin path(s).",
        "group name"
    );

    parser.addOption(pluginPathsOption);
    parser.addOption(pluginGroupNameOption);
    parser.process(app);

    if (!parser.isSet(pluginPathsOption)) {
        qCritical() << "Error: -p is required.";
        parser.showHelp(0);
    }

    auto paths = parser.value(pluginPathsOption);
    auto pluginPaths = paths.split(';', Qt::SkipEmptyParts);

#ifdef QT_DEBUG
    const QDir shellDir(QString("%1/../../plugins/").arg(QCoreApplication::applicationDirPath()));
    if (shellDir.exists()) {
        QCoreApplication::addLibraryPath(shellDir.absolutePath());
    }
#endif

    DLogManager::setLogFormat("%{time}{yy-MM-ddTHH:mm:ss.zzz} [%{type}] [%{category}] <%{function}> %{message}");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    PluginManager pluginManager;
    pluginManager.setPluginPaths(pluginPaths);
    if (!pluginManager.loadPlugins()) {
        qWarning() << "No valid plugins were loaded.";
        return -1;
    }

    QString pluginGroupName;
    if (parser.isSet(pluginPathsOption)) {
        pluginGroupName = parser.value(pluginGroupNameOption);
    }

    if (pluginGroupName.isEmpty()) {
        pluginGroupName = pluginManager.loadedPlugins()[0]->pluginName();
    }

    app.setApplicationName(pluginGroupName);
    app.setApplicationDisplayName(pluginGroupName);
    setproctitle((QStringLiteral("tray plugin: ") + pluginGroupName).toStdString().c_str());
    qunsetenv("QT_SCALE_FACTOR");
    return app.exec();
}
