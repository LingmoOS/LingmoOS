// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "controlcenterdbusadaptor.h"
#include "oceanuimanager.h"

#include <DDBusSender>
#include <DGuiApplicationHelper>
#include <DIconTheme>
#include <DLog>
#include <DPalette>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusConnection>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>
#include <QStringList>
#include <QWindow>

DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

QStringList defaultpath()
{
    const QStringList path{ QStringLiteral(DefaultPluginsDirectory) };
    // , QStringLiteral("/usr/lib/ocean-control-center/modules/") };
    return path;
}

int main(int argc, char *argv[])
{
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::DontSaveApplicationTheme, true);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);

    QGuiApplication *app = new QGuiApplication(argc, argv);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif // (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    app->setOrganizationName("lingmo");
    app->setApplicationName("ocean-control-center");

    // take care of command line options
    QCommandLineOption showOption(QStringList() << "s"
                                                << "show",
                                  "show control center(hide for default).");
    QCommandLineOption toggleOption(QStringList() << "t"
                                                  << "toggle",
                                    "toggle control center visible.");
    QCommandLineOption dbusOption(QStringList() << "d"
                                                << "dbus",
                                  "startup on dbus");
    QCommandLineOption pageOption("p", "specified module page", "page");
    QCommandLineOption pluginDir("spec", "load plugins from specialdir", "plugindir");

    QCommandLineParser parser;
    parser.setApplicationDescription("OCEAN Control Center");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(showOption);
    parser.addOption(toggleOption);
    parser.addOption(dbusOption);
    parser.addOption(pageOption);
    parser.addOption(pluginDir);
    parser.process(*app);

    const QString &reqPage = parser.value(pageOption);
    const QString &refPluginDir = parser.value(pluginDir);

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService(OceanUIDBusService)) {
        qDebug() << "dbus service already registered!"
                 << "pid is:" << qApp->applicationPid();
        if (parser.isSet(toggleOption)) {
            DDBusSender().service(OceanUIDBusService).interface(OceanUIDBusInterface).path(OceanUIDBusPath).method("Toggle").call();
        }

        if (!reqPage.isEmpty()) {
            DDBusSender().service(OceanUIDBusService).interface(OceanUIDBusInterface).path(OceanUIDBusPath).method("ShowPage").arg(reqPage).call();
        } else if (parser.isSet(showOption) && !parser.isSet(dbusOption)) {
            DDBusSender().service(OceanUIDBusService).interface(OceanUIDBusInterface).path(OceanUIDBusPath).method("Show").call();
        }

        return -1;
    }

    DLogManager::setLogFormat("%{time}{yy-MM-ddTHH:mm:ss.zzz} [%{type}] [%{category}] <%{function}:%{line}> %{message}");

    DLogManager::registerJournalAppender();
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

#ifdef CVERSION
    QString verstr(CVERSION);
    if (verstr.isEmpty())
        verstr = "6.0";
    app->setApplicationVersion(verstr);
#else
    app->setApplicationVersion("6.0");
#endif
    app->setWindowIcon(DIconTheme::findQIcon("preferences-system"));
    oceanuiV25::OceanUIManager::installTranslator("ocean-control-center");
    app->setApplicationDisplayName(QObject::tr("Control Center"));
    // app->setApplicationDescription(QApplication::translate("main", "Control Center provides the options for system settings."));

    // QAccessible::installFactory(accessibleFactory);

    oceanuiV25::OceanUIManager *oceanuiManager = new oceanuiV25::OceanUIManager();
    oceanuiManager->init();
    QQmlApplicationEngine *engine = oceanuiManager->engine();
    engine->load(QUrl(QStringLiteral(DefaultModuleDirectory) + "/OceanUIWindow.qml"));
    QList<QObject *> objs = engine->rootObjects();
    for (auto &&obj : objs) {
        QWindow *w = qobject_cast<QWindow *>(obj);
        if (w) {
            oceanuiManager->setMainWindow(w);
            break;
        }
    }
    if (!oceanuiManager->mainWindow()) {
        qWarning() << "Failed to create window";
        return 1;
    }

    oceanuiV25::ControlCenterDBusAdaptor *adaptor = new oceanuiV25::ControlCenterDBusAdaptor(oceanuiManager);
    oceanuiV25::DBusControlCenterGrandSearchService *grandSearchadAptor = new oceanuiV25::DBusControlCenterGrandSearchService(oceanuiManager);

    if (!conn.registerObject(OceanUIDBusPath, oceanuiManager)) {
        qDebug() << "dbus service already registered!"
                 << "pid is:" << qApp->applicationPid();
        return -1;
    }
    if (!refPluginDir.isEmpty()) {
        oceanuiManager->loadModules(true, { refPluginDir });
        adaptor->Show();
    } else {
        oceanuiManager->loadModules(!parser.isSet(dbusOption), defaultpath());
        if (!reqPage.isEmpty()) {
            adaptor->ShowPage(reqPage);
        } else if (parser.isSet(showOption) && !parser.isSet(dbusOption)) {
            adaptor->Show();
        }

#ifdef QT_DEBUG
        // debug时会直接show
        // 发布版本，不会直接显示，为了满足在被dbus调用时，
        // 如果dbus参数错误，不会有任何UI上的变化
        if (1 == argc) {
            DDBusSender().service(OceanUIDBusService).interface(OceanUIDBusInterface).path(OceanUIDBusPath).method("Show").call();
        }
#endif
    }
    int exitCode = app->exec();
    delete oceanuiManager;
    return exitCode;
}
