/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2020 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "partloader.h"

#include "kparts_logging.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KService>
#include <KSharedConfig>

#include <QJsonArray>
#include <QMetaEnum>
#include <QMimeDatabase>
#include <QMimeType>

static QList<KPluginMetaData> partsFromUserPreference(const QString &mimeType)
{
    auto config = KSharedConfig::openConfig(QStringLiteral("kpartsrc"), KConfig::NoGlobals);
    const QStringList pluginIds = config->group(QStringLiteral("Added KDE Part Associations")).readXdgListEntry(mimeType);
    QList<KPluginMetaData> plugins;
    plugins.reserve(pluginIds.size());
    for (const QString &pluginId : pluginIds) {
        if (KPluginMetaData data(QLatin1String("kf6/parts/") + pluginId); data.isValid()) {
            plugins << data;
        }
    }
    return plugins;
}

// A plugin can support N mimetypes. Pick the one that is closest to @parent in the inheritance tree
// and return how far it is from that parent (0 = same mimetype, 1 = direct child, etc.)
static int pluginDistanceToMimeType(const KPluginMetaData &md, const QString &parent)
{
    QMimeDatabase db;
    auto distanceToMimeType = [&](const QString &mime) {
        if (mime == parent) {
            return 0;
        }
        const QStringList ancestors = db.mimeTypeForName(mime).allAncestors();
        const int dist = ancestors.indexOf(parent);
        return dist == -1 ? 50 : dist + 1;
    };
    const QStringList mimes = md.mimeTypes();
    int minDistance = 50;
    for (const QString &mime : mimes) {
        minDistance = std::min(minDistance, distanceToMimeType(mime));
    }
    return minDistance;
}

KParts::PartCapabilities KParts::PartLoader::partCapabilities(const KPluginMetaData &data)
{
    QJsonValue capsArrayRaw = data.rawData().value(QLatin1String("KParts")).toObject().value(QLatin1String("Capabilities"));
    KParts::PartCapabilities parsedCapabilties = {};
    const static QMetaEnum metaEnum = QMetaEnum::fromType<KParts::PartCapability>();
    QJsonArray capabilities = capsArrayRaw.toArray();
    for (const QJsonValue &capability : capabilities) {
        bool ok = true;
        PartCapability parsedCapability = (PartCapability)metaEnum.keyToValue(capability.toString().toLocal8Bit().constData(), &ok);
        if (ok) {
            parsedCapabilties |= parsedCapability;
        } else {
            qCWarning(KPARTSLOG) << "Could not find capability value" << capability.toString().toLocal8Bit().constData();
        }
    }

    // Don't bother looking at fallback API
    if (!capsArrayRaw.isUndefined()) {
        return parsedCapabilties;
    }

    static QMap<QString, KParts::PartCapability> capabilityMapping = {
        {QStringLiteral("KParts/ReadOnlyPart"), PartCapability::ReadOnly},
        {QStringLiteral("KParts/ReadWritePart"), PartCapability::ReadWrite},
        {QStringLiteral("Browser/View"), PartCapability::BrowserView},
    };
    const auto serviceTypes = data.rawData().value(QLatin1String("KPlugin")).toObject().value(QLatin1String("ServiceTypes")).toVariant().toStringList();
    if (!serviceTypes.isEmpty()) {
        qCWarning(KPARTSLOG) << data
                             << "still defined ServiceTypes - this is deprecated in favor of providing a "
                                " \"Capabilities\" list in the \"KParts\" object in the root of the metadata";
        for (const QString &serviceType : serviceTypes) {
            auto it = capabilityMapping.find(serviceType);
            if (it == capabilityMapping.cend()) {
                qCWarning(KPARTSLOG) << "ServiceType" << serviceType << "from" << data
                                     << "is not a known value that can be mapped to new Capability enum values";
            } else {
                parsedCapabilties |= *it;
            }
        }
    }
    return parsedCapabilties;
}

QList<KPluginMetaData> KParts::PartLoader::partsForMimeType(const QString &mimeType)
{
    auto supportsMime = [&mimeType](const KPluginMetaData &md) {
        if (md.supportsMimeType(mimeType)) {
            return true;
        }
        auto pluginJson = md.rawData();
        auto pluginNamespace = pluginJson.value(QLatin1String("KParts")).toObject().value(QLatin1String("PluginNamespace")).toString();
        if (pluginNamespace.isEmpty()) {
            return false;
        }
        auto plugins = KPluginMetaData::findPlugins(pluginNamespace, [&mimeType](const KPluginMetaData &pluginMd) {
            return pluginMd.supportsMimeType(mimeType);
        });
        return !plugins.isEmpty();
    };
    QList<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("kf6/parts"), supportsMime);
    auto orderPredicate = [&](const KPluginMetaData &left, const KPluginMetaData &right) {
        // We filtered based on "supports mimetype", but this didn't order from most-specific to least-specific.
        const int leftDistance = pluginDistanceToMimeType(left, mimeType);
        const int rightDistance = pluginDistanceToMimeType(right, mimeType);
        if (leftDistance < rightDistance) {
            return true;
        }
        if (leftDistance > rightDistance) {
            return false;
        }
        // Plugins who support the same mimetype are then sorted by initial preference
        const auto getInitialPreference = [](const KPluginMetaData &data) {
            const QJsonObject obj = data.rawData();
            if (const QJsonValue initialPref = obj.value(QLatin1String("KParts")).toObject().value(QLatin1String("InitialPreference"));
                !initialPref.isUndefined()) {
                return initialPref.toInt();
            }
            return data.rawData().value(QLatin1String("KPlugin")).toObject().value(QLatin1String("InitialPreference")).toInt();
        };
        return getInitialPreference(left) > getInitialPreference(right);
    };
    std::sort(plugins.begin(), plugins.end(), orderPredicate);

    const QList<KPluginMetaData> userParts = partsFromUserPreference(mimeType);
    if (!userParts.isEmpty()) {
        plugins = userParts;
    }
    return plugins;
}

void KParts::PartLoader::Private::getErrorStrings(QString *errorString, QString *errorText, const QString &argument, ErrorType type)
{
    switch (type) {
    case CouldNotLoadPlugin:
        *errorString = i18n("KPluginFactory could not load the plugin: %1", argument);
        *errorText = QStringLiteral("KPluginFactory could not load the plugin: %1").arg(argument);
        break;
    case NoPartFoundForMimeType:
        *errorString = i18n("No part was found for mimeType %1", argument);
        *errorText = QStringLiteral("No part was found for mimeType %1").arg(argument);
        break;
    case NoPartInstantiatedForMimeType:
        *errorString = i18n("No part could be instantiated for mimeType %1", argument);
        *errorText = QStringLiteral("No part could be instantiated for mimeType %1").arg(argument);
        break;
    default:
        qCWarning(KPARTSLOG) << "PartLoader::Private::getErrorStrings got unexpected error type" << type;
        break;
    }
};

#include "moc_partloader.cpp"
