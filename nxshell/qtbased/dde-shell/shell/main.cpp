// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStandardPaths>

#include <DLog>
#include <DGuiApplicationHelper>
#include <QWindow>

#include <csignal>
#include <iostream>

#include "applet.h"
#include "containment.h"
#include "pluginloader.h"
#include "appletloader.h"
#include "qmlengine.h"
#include "shell.h"

DS_USE_NAMESPACE
DGUI_USE_NAMESPACE

DS_BEGIN_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(dsLog)
DS_END_NAMESPACE

void outputPluginTreeStruct(const DPluginMetaData &plugin, int level)
{
    const QString separator(level * 4, ' ');
    std::cout << qPrintable(separator + plugin.pluginId()) << std::endl;
    for (const auto &item : DPluginLoader::instance()->childrenPlugin(plugin.pluginId())) {
        outputPluginTreeStruct(item, level + 1);
    }
}

// disable log's output in some case.
static void disableLogOutput()
{
    QLoggingCategory::setFilterRules("*.debug=false");
}

static void exitApp(int signal)
{
    Q_UNUSED(signal);
    QCoreApplication::exit();
}

class AppletManager
{
public:
    explicit AppletManager(const QStringList &pluginIds)
    {
        qCDebug(dsLog) << "Preloading plugins:" << pluginIds;
        auto rootApplet = qobject_cast<DContainment *>(DPluginLoader::instance()->rootApplet());
        Q_ASSERT(rootApplet);

        for (const auto &pluginId : pluginIds) {
            auto applet = rootApplet->createApplet(DAppletData{pluginId});
            if (!applet) {
                qCWarning(dsLog) << "Loading plugin failed:" << pluginId;
                continue;
            }

            auto loader = new DAppletLoader(applet);
            m_loaders << loader;

            QObject::connect(loader, &DAppletLoader::failed, qApp, [this, loader, pluginIds](const QString &pluginId) {
                if (pluginIds.contains(pluginId)) {
                    m_loaders.removeOne(loader);
                    loader->deleteLater();
                }
            });
        }
    }
    void enableSceneview()
    {
        auto rootApplet = qobject_cast<DContainment *>(DPluginLoader::instance()->rootApplet());
        rootApplet->setRootObject(DQmlEngine::createObject(QUrl("qrc:/shell/SceneWindow.qml")));
    }
    void exec()
    {
        for (auto loader : std::as_const(m_loaders)) {
            loader->exec();
        }
    }
    void quit()
    {
        for (auto item : std::as_const(m_loaders)) {
            item->deleteLater();
        }
    }
    QList<DAppletLoader *> m_loaders;
};

int main(int argc, char *argv[])
{
    setenv("DSG_APP_ID", "org.deepin.dde.shell", 0);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    QApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("org.deepin.dde-shell");
    a.setApplicationVersion(QT_STRINGIFY(DS_VERSION));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption panelOption("p", "collections of panel.", "panel", QString());
    parser.addOption(panelOption);
    QCommandLineOption categoryOption("C", "collections of root panels by category.", "category", QString());
    parser.addOption(categoryOption);
    QCommandLineOption testOption(QStringList() << "t" << "test", "application test.");
    parser.addOption(testOption);
    QCommandLineOption disableAppletOption("d", "disabled applet.", "disable-applet", QString());
    parser.addOption(disableAppletOption);
    QCommandLineOption listOption("list", "List all applets.", QString());
    parser.addOption(listOption);
    QCommandLineOption sceneviewOption("sceneview", "View applets in scene, it only works without Window.", QString());
    parser.addOption(sceneviewOption);

    parser.process(a);

    if (parser.isSet(listOption)) {
        disableLogOutput();
        for (const auto &item : DPluginLoader::instance()->rootPlugins()) {
            outputPluginTreeStruct(item, 0);
        }
        return 0;
    }

    auto dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QStringList fallbacks = QIcon::fallbackSearchPaths();
    for (const auto fb : dirs) {
        fallbacks << fb + QLatin1String("/icons");
    }
    // To Fix (developer-center#8413) Qt6 find icons will ignore ${GenericDataLocation}/icons/xxx.png
    QIcon::setFallbackSearchPaths(fallbacks);

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();
    Dtk::Core::DLogManager::registerJournalAppender();
    qCInfo(dsLog) << "Log path is:" << Dtk::Core::DLogManager::getlogFilePath();

    // add signal handler, and call QCoreApplication::exit.
    std::signal(SIGINT, exitApp);
    std::signal(SIGABRT, exitApp);
    std::signal(SIGTERM, exitApp);
    std::signal(SIGKILL, exitApp);

    QList<QString> pluginIds;
    if (parser.isSet(testOption)) {
        pluginIds << "org.deepin.ds.example";
    } else if (parser.isSet(panelOption)) {
        pluginIds << parser.values(panelOption);
    } else if (parser.isSet(categoryOption)) {
        const QList<QString> &categories = parser.values(categoryOption);
        for (const auto &item : DPluginLoader::instance()->rootPlugins()) {
            const auto catetroy = item.value("Category").toString();
            if (catetroy.isEmpty())
                continue;
            if (categories.contains(catetroy)) {
                pluginIds << item.pluginId();
            }
        }

    } else {
        for (const auto &item : DPluginLoader::instance()->rootPlugins()) {
            pluginIds << item.pluginId();
        }
    }
    if (parser.isSet(disableAppletOption)) {
        const auto disabledApplets = parser.values(disableAppletOption);
        DPluginLoader::instance()->setDisabledApplets(disabledApplets);
        pluginIds.removeIf([disabledApplets] (const QString &item) {
            return disabledApplets.contains(item);
        });
    }

    Shell shell;
    shell.dconfigsMigrate();
    // TODO disable qml's cache avoid to parsing error for ExecutionEngine.
    shell.disableQmlCache();
    shell.setFlickableWheelDeceleration(6000);

    AppletManager manager(pluginIds);
    if (parser.isSet(sceneviewOption))
        manager.enableSceneview();

    QMetaObject::invokeMethod(&a, [&manager](){
        manager.exec();
    }, Qt::QueuedConnection);

    QObject::connect(qApp, &QCoreApplication::aboutToQuit, qApp, [&manager]() {
        qCInfo(dsLog) << "Exit dde-shell.";
        DPluginLoader::instance()->destroy();
        manager.quit();
    });

    return a.exec();
}
