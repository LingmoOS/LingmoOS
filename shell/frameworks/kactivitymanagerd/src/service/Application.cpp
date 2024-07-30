/*
 *   SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

// Self
#include "Application.h"
#include <kactivities-features.h>

// Qt
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QThread>

// KDE
#include <KCrash>
#include <KPluginMetaData>
#include <kdbusservice.h>
#include <ksharedconfig.h>

// utils
#include <utils/d_ptr_implementation.h>

// System
#include <functional>
#include <memory>
#include <signal.h>
#include <stdlib.h>

// Local
#include "Activities.h"
#include "Config.h"
#include "DebugApplication.h"
#include "Features.h"
#include "Plugin.h"
#include "Resources.h"
#include "common/dbus/common.h"

namespace
{
QList<QThread *> s_moduleThreads;
}

// Runs a QObject inside a QThread

template<typename T>
T *runInQThread()
{
    T *object = new T();

    class Thread : public QThread
    {
    public:
        Thread(T *ptr = nullptr)
            : QThread()
            , object(ptr)
        {
        }

        void run() override
        {
            std::unique_ptr<T> o(object);
            exec();
        }

    private:
        T *object;

    } *thread = new Thread(object);

    s_moduleThreads << thread;

    object->moveToThread(thread);
    thread->start();

    return object;
}

class Application::Private
{
public:
    Private()
    {
    }

    bool loadPlugin(const KPluginMetaData &plugin);

    Resources *resources;
    Activities *activities;
    Features *features;

    QStringList pluginIds;
    QList<Plugin *> plugins;

    static Application *s_instance;
};

Application *Application::Private::s_instance = nullptr;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
{
}

void Application::init()
{
    // KAMD is a daemon, if it crashes it is not a problem as
    // long as it restarts properly
    // TODO: Restart on crash
    //       KCrash::setFlags(KCrash::AutoRestart);
    d->resources = runInQThread<Resources>();
    d->activities = runInQThread<Activities>();
    d->features = runInQThread<Features>();
    /* d->config */ new Config(this); // this does not need a separate thread

    QMetaObject::invokeMethod(this, "loadPlugins", Qt::QueuedConnection);

    QDBusConnection::sessionBus().registerObject(QStringLiteral("/ActivityManager"), this, QDBusConnection::ExportAllSlots);

    if (!QDBusConnection::sessionBus().registerService(KAMD_DBUS_SERVICE)) {
        QCoreApplication::exit(EXIT_SUCCESS);
    }
}

bool Application::Private::loadPlugin(const KPluginMetaData &plugin)
{
    if (!plugin.isValid()) {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] plugin offer not valid";
        return false;
    }

    if (pluginIds.contains(plugin.pluginId())) {
        qCDebug(KAMD_LOG_APPLICATION) << "[   OK   ] already loaded:  " << plugin.pluginId();
        return true;
    }

    const auto result = KPluginFactory::instantiatePlugin<Plugin>(plugin);

    if (!result) {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] Could not load plugin:" << plugin.pluginId() << result.errorText;
        return false;
    }

    auto pluginInstance = result.plugin;

    auto &modules = Module::get();

    bool success = pluginInstance->init(modules);

    if (success) {
        pluginIds << plugin.pluginId();
        plugins << pluginInstance;

        qCDebug(KAMD_LOG_APPLICATION) << "[   OK   ] loaded:  " << plugin.pluginId();
        return true;
    } else {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] init: " << plugin.pluginId();
        // TODO: Show a notification for a plugin that failed to load
        delete pluginInstance;
        return false;
    }
}

void Application::loadPlugins()
{
    using namespace std::placeholders;

    const auto offers = KPluginMetaData::findPlugins(QStringLiteral(KAMD_PLUGIN_DIR), {}, KPluginMetaData::AllowEmptyMetaData);
    qCDebug(KAMD_LOG_APPLICATION) << "Found" << offers.size() << "enabled plugins:";

    for (const auto &offer : offers) {
        d->loadPlugin(offer);
    }
}

bool Application::loadPlugin(const QString &pluginId)
{
    auto offer = KPluginMetaData::findPluginById(QStringLiteral(KAMD_PLUGIN_DIR), pluginId, KPluginMetaData::AllowEmptyMetaData);
    if (!offer.isValid()) {
        qCWarning(KAMD_LOG_APPLICATION) << "[ FAILED ] not found: " << pluginId;
        return false;
    }

    return d->loadPlugin(offer);
}

Application::~Application()
{
    qCDebug(KAMD_LOG_APPLICATION) << "Cleaning up...";

    // Waiting for the threads to finish
    for (const auto thread : s_moduleThreads) {
        thread->quit();
        thread->wait();

        delete thread;
    }

    // Deleting plugin objects
    for (const auto plugin : d->plugins) {
        delete plugin;
    }

    Private::s_instance = nullptr;
}

int Application::newInstance()
{
    // We don't want to show the mainWindow()
    return 0;
}

Activities &Application::activities() const
{
    return *d->activities;
}

Resources &Application::resources() const
{
    return *d->resources;
}

// void Application::quit()
// {
//     if (Private::s_instance) {
//         Private::s_instance->exit();
//         delete Private::s_instance;
//     }
// }

void Application::quit()
{
    QApplication::quit();
}

#include "Version.h"
QString Application::serviceVersion() const
{
    return KACTIVITIES_VERSION_STRING;
}

int main(int argc, char **argv)
{
    // Disable session management for this process
    qunsetenv("SESSION_MANAGER");

    QGuiApplication::setDesktopSettingsAware(false);

    Application application(argc, argv);
    application.setApplicationName(QStringLiteral("ActivityManager"));
    application.setOrganizationDomain(QStringLiteral("kde.org"));

    KCrash::initialize();

    application.init();

    return application.exec();
}

QStringList Application::loadedPlugins() const
{
    return d->pluginIds;
}

#include "moc_Application.cpp"
