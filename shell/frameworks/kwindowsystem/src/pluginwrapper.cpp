/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "kwindoweffects_dummy_p.h"
#include "kwindowshadow_dummy_p.h"
#include "kwindowsystem_debug.h"
#include "kwindowsystem_dummy_p.h"
#include "kwindowsystemplugininterface_p.h"
#include "pluginwrapper_p.h"

#include <QDir>
#include <QGuiApplication>
#include <QJsonArray>
#include <QLibrary>
#include <QPluginLoader>

Q_GLOBAL_STATIC(KWindowSystemPluginWrapper, s_pluginWrapper)

static QStringList pluginCandidates()
{
    QStringList ret;
    const auto paths = QCoreApplication::libraryPaths();
    for (const QString &path : paths) {
        static const QStringList searchFolders{
            QStringLiteral("/kf6/org.kde.kwindowsystem.platforms"),
            QStringLiteral("/kf6/kwindowsystem"),
        };
        for (const QString &searchFolder : searchFolders) {
            QDir pluginDir(path + searchFolder);
            if (!pluginDir.exists()) {
                continue;
            }
            const auto entries = pluginDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
            for (const QString &entry : entries) {
                ret << pluginDir.absoluteFilePath(entry);
            }
        }
    }
    return ret;
}

static bool checkPlatform(const QJsonObject &metadata, const QString &platformName)
{
    const QJsonArray platforms = metadata.value(QStringLiteral("MetaData")).toObject().value(QStringLiteral("platforms")).toArray();
    return std::any_of(platforms.begin(), platforms.end(), [&platformName](const QJsonValue &value) {
        return QString::compare(platformName, value.toString(), Qt::CaseInsensitive) == 0;
    });
}

static KWindowSystemPluginInterface *loadPlugin()
{
    if (!qobject_cast<QGuiApplication *>(QCoreApplication::instance())) {
        qCWarning(LOG_KWINDOWSYSTEM) << "Cannot use KWindowSystem without a QGuiApplication";
        return nullptr;
    }

    QString platformName = QGuiApplication::platformName();
    if (platformName == QLatin1String("flatpak")) {
        // here we cannot know what is the actual windowing system, let's try it's env variable
        const auto flatpakPlatform = QString::fromLocal8Bit(qgetenv("QT_QPA_FLATPAK_PLATFORM"));
        if (!flatpakPlatform.isEmpty()) {
            platformName = flatpakPlatform;
        }
    }

    const QList<QStaticPlugin> staticPlugins = QPluginLoader::staticPlugins();
    for (const QStaticPlugin &staticPlugin : staticPlugins) {
        const QJsonObject metadata = staticPlugin.metaData();
        if (metadata.value(QLatin1String("IID")) != QLatin1String(KWindowSystemPluginInterface_iid)) {
            continue;
        }
        if (checkPlatform(metadata, platformName)) {
            KWindowSystemPluginInterface *interface = qobject_cast<KWindowSystemPluginInterface *>(staticPlugin.instance());
            if (interface) {
                qCDebug(LOG_KWINDOWSYSTEM) << "Loaded a static plugin for platform" << platformName;
                return interface;
            }
        }
    }

    const auto candidates = pluginCandidates();
    for (const QString &candidate : candidates) {
        if (!QLibrary::isLibrary(candidate)) {
            continue;
        }
        QPluginLoader loader(candidate);
        if (checkPlatform(loader.metaData(), platformName)) {
            KWindowSystemPluginInterface *interface = qobject_cast<KWindowSystemPluginInterface *>(loader.instance());
            if (interface) {
                qCDebug(LOG_KWINDOWSYSTEM) << "Loaded plugin" << candidate << "for platform" << platformName;
                return interface;
            }
        }
    }

    qCWarning(LOG_KWINDOWSYSTEM) << "Could not find any platform plugin";
    return nullptr;
}

KWindowSystemPluginWrapper::KWindowSystemPluginWrapper()
    : m_plugin(loadPlugin())
    , m_effects()
{
    if (m_plugin) {
        m_effects.reset(m_plugin->createEffects());
    }
    if (!m_effects) {
        m_effects.reset(new KWindowEffectsPrivateDummy());
    }
}

KWindowSystemPluginWrapper::~KWindowSystemPluginWrapper()
{
}

KWindowEffectsPrivate *KWindowSystemPluginWrapper::effects() const
{
    return m_effects.get();
}

KWindowSystemPrivate *KWindowSystemPluginWrapper::createWindowSystem() const
{
    KWindowSystemPrivate *p = nullptr;
    if (m_plugin) {
        p = m_plugin->createWindowSystem();
    }
    if (!p) {
        p = new KWindowSystemPrivateDummy();
    }
    return p;
}

KWindowShadowPrivate *KWindowSystemPluginWrapper::createWindowShadow() const
{
    KWindowShadowPrivate *p = nullptr;
    if (m_plugin) {
        p = m_plugin->createWindowShadow();
    }
    if (!p) {
        p = new KWindowShadowPrivateDummy();
    }
    return p;
}

KWindowShadowTilePrivate *KWindowSystemPluginWrapper::createWindowShadowTile() const
{
    KWindowShadowTilePrivate *p = nullptr;
    if (m_plugin) {
        p = m_plugin->createWindowShadowTile();
    }
    if (!p) {
        p = new KWindowShadowTilePrivateDummy();
    }
    return p;
}

const KWindowSystemPluginWrapper &KWindowSystemPluginWrapper::self()
{
    return *s_pluginWrapper;
}
