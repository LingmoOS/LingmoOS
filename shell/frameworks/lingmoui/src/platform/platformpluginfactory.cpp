/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "platformpluginfactory.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QQuickStyle>

#include "lingmouiplatform_logging.h"

namespace LingmoUI
{
namespace Platform
{

PlatformPluginFactory::PlatformPluginFactory(QObject *parent)
    : QObject(parent)
{
}

PlatformPluginFactory::~PlatformPluginFactory() = default;

PlatformPluginFactory *PlatformPluginFactory::findPlugin(const QString &preferredName)
{
    static QHash<QString, PlatformPluginFactory *> factories = QHash<QString, PlatformPluginFactory *>();

    QString pluginName = preferredName.isEmpty() ? QQuickStyle::name() : preferredName;
    // check for the plugin only once: it's an heavy operation
    if (auto it = factories.constFind(pluginName); it != factories.constEnd()) {
        return it.value();
    }

    // Even plugins that aren't found are in the map, so we know we shouldn't check again withthis expensive operation
    factories[pluginName] = nullptr;

#ifdef LINGMOUI_BUILD_TYPE_STATIC
    for (QObject *staticPlugin : QPluginLoader::staticInstances()) {
        PlatformPluginFactory *factory = qobject_cast<PlatformPluginFactory *>(staticPlugin);
        if (factory) {
            factories[pluginName] = factory;
            break;
        }
    }
#else
    const auto libraryPaths = QCoreApplication::libraryPaths();
    for (const QString &path : libraryPaths) {

#ifdef Q_OS_ANDROID
        const QDir dir(path);
#else
        const QDir dir(path + QStringLiteral("/kf6/lingmoui/platform"));
#endif

        const auto fileNames = dir.entryList(QDir::Files);

        for (const QString &fileName : fileNames) {

#ifdef Q_OS_ANDROID
            if (fileName.startsWith(QStringLiteral("libplugins_kf6_lingmoui_platform_")) && QLibrary::isLibrary(fileName)) {
#endif
                if (!pluginName.isEmpty() && fileName.contains(pluginName)) {
                    // TODO: env variable too?
                    QPluginLoader loader(dir.absoluteFilePath(fileName));
                    QObject *plugin = loader.instance();
                    // TODO: load actually a factory as plugin

                    qCDebug(LingmoUIPlatform) << "Loading style plugin from" << dir.absoluteFilePath(fileName);

                    if (auto factory = qobject_cast<PlatformPluginFactory *>(plugin)) {
                        factories[pluginName] = factory;
                        break;
                    }
                }
#ifdef Q_OS_ANDROID
            }
#endif
        }

        // Ensure we only load the first plugin from the first plugin location.
        // If we do not break here, we may end up loading a different plugin
        // in place of the first one.
        if (factories.value(pluginName) != nullptr) {
            break;
        }
    }
#endif

    PlatformPluginFactory *factory = factories.value(pluginName);

    if (factory == nullptr) {
        qWarning(LingmoUIPlatform) << "Failed to find a LingmoUI platform plugin for style" << QQuickStyle::name();
    }

    return factory;
}

}
}

#include "moc_platformpluginfactory.cpp"
