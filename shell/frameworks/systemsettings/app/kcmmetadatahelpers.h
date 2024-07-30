/*
 *   SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KAuthorized>
#include <KCModuleData>
#include <KFileUtils>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KService>

#include <QGuiApplication>
#include <QStandardPaths>

#include <set>

#include "../systemsettings_app_debug.h"

enum MetaDataSource {
    SystemSettings = 1,
    KInfoCenter = 2,
    All = SystemSettings | KInfoCenter,
};

inline QList<KPluginMetaData> findExternalKCMModules(MetaDataSource source)
{
    const auto findExternalModulesInFilesystem = [](const QString &sourceNamespace) {
        const QString sourceNamespaceDirName = QStringLiteral("lingmo/%1/externalmodules").arg(sourceNamespace);
        const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, sourceNamespaceDirName, QStandardPaths::LocateDirectory);
        const QStringList files = KFileUtils::findAllUniqueFiles(dirs, QStringList{QStringLiteral("*.desktop")});

        QList<KPluginMetaData> metaDataList;
        for (const QString &file : files) {
            KService service(file);
            QJsonObject kplugin;
            kplugin.insert(QLatin1String("Name"), service.name());
            kplugin.insert(QLatin1String("Icon"), service.icon());
            kplugin.insert(QLatin1String("Description"), service.comment());

            QJsonObject root;
            root.insert(QLatin1String("KPlugin"), kplugin);
            root.insert(QLatin1String("X-KDE-Weight"), service.property<int>(QStringLiteral("X-KDE-Weight")));
            root.insert(QLatin1String("X-KDE-KInfoCenter-Category"), service.property<QString>(QStringLiteral("X-KDE-KInfoCenter-Category")));
            root.insert(QLatin1String("X-KDE-System-Settings-Category"), service.property<QString>(QStringLiteral("X-KDE-System-Settings-Category")));
            root.insert(QLatin1String("IsExternalApp"), true);

            metaDataList << KPluginMetaData(root, file);
        }
        return metaDataList;
    };

    QList<KPluginMetaData> metaDataList;
    if (source & SystemSettings) {
        metaDataList << findExternalModulesInFilesystem(QStringLiteral("systemsettings"));
    }

    if (source & KInfoCenter) {
        metaDataList << findExternalModulesInFilesystem(QStringLiteral("kinfocenter"));
    }

    return metaDataList;
}

inline QList<KPluginMetaData> findKCMsMetaData(MetaDataSource source)
{
    QList<KPluginMetaData> modules;
    std::set<QString> uniquePluginIds;

    auto filter = [](const KPluginMetaData &data) {
        const auto supportedPlatforms = data.value(QStringLiteral("X-KDE-OnlyShowOnQtPlatforms"), QStringList());
        return supportedPlatforms.isEmpty() || supportedPlatforms.contains(qGuiApp->platformName());
    };

    // We need the exist calls because otherwise the trader language aborts if the property doesn't exist and the second part of the or is not evaluated
    QList<KPluginMetaData> metaDataList = KPluginMetaData::findPlugins(QStringLiteral("lingmo/kcms"), filter);
    if (source & SystemSettings) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("lingmo/kcms/systemsettings"), filter);
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("lingmo/kcms/systemsettings_qwidgets"), filter);
    }
    if (source & KInfoCenter) {
        metaDataList << KPluginMetaData::findPlugins(QStringLiteral("lingmo/kcms/kinfocenter"), filter);
    }
    for (const auto &m : std::as_const(metaDataList)) {
        // We check both since porting a module to loading view KPluginMetaData drops ".desktop" from the pluginId()
        if (!KAuthorized::authorizeControlModule(m.pluginId())) {
            continue;
        }
        modules << m;
        const bool inserted = uniquePluginIds.insert(m.pluginId()).second;
        if (!inserted) {
            qWarning() << "the plugin" << m.pluginId() << " was found in multiple namespaces";
        }
    }
    std::stable_sort(modules.begin(), modules.end(), [](const KPluginMetaData &m1, const KPluginMetaData &m2) {
        return QString::compare(m1.pluginId(), m2.pluginId(), Qt::CaseInsensitive) < 0;
    });
    return modules;
}

inline bool isKinfoCenterKcm(const KPluginMetaData &data)
{
    // external module or a KCM in the namespace
    return data.fileName().contains(QLatin1String("/kinfocenter/"));
}

inline KCModuleData *loadModuleData(const KPluginMetaData &data)
{
    if (auto factory = KPluginFactory::loadFactory(data).plugin) {
        return factory->create<KCModuleData>();
    }
    return nullptr;
}
