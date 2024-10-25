/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "pluginloader.h"

#include <QDebug>
#include <QGuiApplication>
#include <QPluginLoader>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>

#include <KLazyLocalizedString>
#include <KPackage/PackageLoader>
#include <KRuntimePlatform>

#include "config-lingmo.h"

#include "applet.h"
#include "containment.h"
#include "containmentactions.h"
#include "debug_p.h"
#include "private/applet_p.h"

namespace Lingmo
{
inline bool isContainmentMetaData(const KPluginMetaData &md)
{
    return md.rawData().contains(QStringLiteral("X-Lingmo-ContainmentType"));
}

PluginLoader *PluginLoader::self()
{
    static PluginLoader self;
    return &self;
}

Applet *PluginLoader::loadApplet(const QString &name, uint appletId, const QVariantList &args)
{
    if (name.isEmpty()) {
        return nullptr;
    }

    Applet *applet = nullptr;

    if (appletId == 0) {
        appletId = ++AppletPrivate::s_maxAppletId;
    }

    // name can be either an actual applet name or an absolute path, in the
    // latter case, ensure we only use the name part of the path.
    const QString pluginName = name.section(QLatin1Char('/'), -1);

    KPluginMetaData plugin(QStringLiteral("lingmo/applets/") + pluginName, KPluginMetaData::AllowEmptyMetaData);
    const KPackage::Package p = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Applet"), name);

    if (!p.isValid()) {
        qWarning(LOG_LINGMO) << "Applet invalid: Cannot find a package for" << name;
    }

    // If the applet is using another applet package, search for the plugin of the other applet
    if (!plugin.isValid()) {
        const QString parentPlugin = p.metadata().value(QStringLiteral("X-Lingmo-RootPath"));
        if (!parentPlugin.isEmpty()) {
            plugin = KPluginMetaData(QStringLiteral("lingmo/applets/") + parentPlugin, KPluginMetaData::AllowEmptyMetaData);
        }
    }

    if (plugin.isValid()) {
        QVariantList allArgs = QVariantList{QVariant::fromValue(p), appletId} << args;
        if (KPluginFactory *factory = KPluginFactory::loadFactory(plugin).plugin) {
            if (factory->metaData().rawData().isEmpty()) {
                factory->setMetaData(p.metadata());
            }
            applet = factory->create<Lingmo::Applet>(nullptr, allArgs);
        }
    }
    if (applet) {
        return applet;
    }

    QVariantList allArgs;
    allArgs << QVariant::fromValue(p) << appletId << args;

    if (isContainmentMetaData(p.metadata())) {
        applet = new Containment(nullptr, p.metadata(), allArgs);
    } else {
        KPluginMetaData metadata = p.metadata();
        if (metadata.pluginId().isEmpty()) {
            // Add fake extension to parse completeBaseName() as pluginId
            // without having to construct a fake JSON metadata object.
            // This would help with better error messages which would
            // at least show the missing applet's ID.
            const auto fakeFileName = name + u'.';
            metadata = KPluginMetaData(QJsonObject(), fakeFileName);
        }
        applet = new Applet(nullptr, metadata, allArgs);
    }

    const QString localePath = p.filePath("translations");
    if (!localePath.isEmpty()) {
        KLocalizedString::addDomainLocaleDir(QByteArray("lingmo_applet_") + name.toLatin1(), localePath);
    }
    return applet;
}

ContainmentActions *PluginLoader::loadContainmentActions(Containment *parent, const QString &name, const QVariantList &args)
{
    if (name.isEmpty()) {
        return nullptr;
    }

    KPluginMetaData plugin(QStringLiteral("lingmo/containmentactions/") + name, KPluginMetaData::AllowEmptyMetaData);

    if (plugin.isValid()) {
        if (auto res = KPluginFactory::instantiatePlugin<Lingmo::ContainmentActions>(plugin, nullptr, {QVariant::fromValue(plugin)})) {
            return res.plugin;
        }
    }

    return nullptr;
}

QList<KPluginMetaData> PluginLoader::listAppletMetaData(const QString &category)
{
    auto platforms = KRuntimePlatform::runtimePlatform();
    // For now desktop always lists everything
    if (platforms.contains(QStringLiteral("desktop"))) {
        platforms.clear();
    }

    // FIXME: this assumes we are always use packages.. no pure c++
    std::function<bool(const KPluginMetaData &)> filter;
    if (category.isEmpty()) { // use all but the excluded categories
        KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("General"));
        QStringList excluded = group.readEntry("ExcludeCategories", QStringList());

        filter = [excluded, platforms](const KPluginMetaData &md) -> bool {
            if (!platforms.isEmpty() && !md.formFactors().isEmpty()) {
                bool found = false;
                for (const auto &plat : platforms) {
                    if (md.formFactors().contains(plat)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            return !excluded.contains(md.category());
        };
    } else { // specific category (this could be an excluded one - is that bad?)

        filter = [category, platforms](const KPluginMetaData &md) -> bool {
            if (!platforms.isEmpty() && !md.formFactors().isEmpty()) {
                bool found = false;
                for (const auto &plat : platforms) {
                    if (md.formFactors().contains(plat)) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

            if (category == QLatin1String("Miscellaneous")) {
                return md.category() == category || md.category().isEmpty();
            } else {
                return md.category() == category;
            }
        };
    }

    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Lingmo/Applet"), QString(), filter);
}

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForMimeType(const QString &mimeType)
{
    auto filter = [&mimeType](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-Lingmo-DropMimeTypes"), QStringList()).contains(mimeType);
    };
    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Lingmo/Applet"), QString(), filter);
}

QList<KPluginMetaData> PluginLoader::listAppletMetaDataForUrl(const QUrl &url)
{
    auto filter = [](const KPluginMetaData &md) -> bool {
        return !md.value(QStringLiteral("X-Lingmo-DropUrlPatterns"), QStringList()).isEmpty();
    };
    const QList<KPluginMetaData> allApplets = KPackage::PackageLoader::self()->findPackages(QStringLiteral("Lingmo/Applet"), QString(), filter);

    QList<KPluginMetaData> filtered;
    for (const KPluginMetaData &md : allApplets) {
        const QStringList urlPatterns = md.value(QStringLiteral("X-Lingmo-DropUrlPatterns"), QStringList());
        for (const QString &glob : urlPatterns) {
            QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(glob)));
            if (rx.match(url.toString()).hasMatch()) {
                filtered << md;
            }
        }
    }

    return filtered;
}

QList<KPluginMetaData> PluginLoader::listContainmentsMetaData(std::function<bool(const KPluginMetaData &)> filter)
{
    auto ownFilter = [filter](const KPluginMetaData &md) -> bool {
        return isContainmentMetaData(md) && filter(md);
    };

    return KPackage::PackageLoader::self()->findPackages(QStringLiteral("Lingmo/Applet"), QString(), ownFilter);
}

QList<KPluginMetaData> PluginLoader::listContainmentsMetaDataOfType(const QString &type)
{
    auto filter = [type](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-Lingmo-ContainmentType")) == type;
    };

    return listContainmentsMetaData(filter);
}

QList<KPluginMetaData> PluginLoader::listContainmentActionsMetaData(const QString &parentApp)
{
    auto filter = [&parentApp](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-KDE-ParentApp")) == parentApp;
    };

    QList<KPluginMetaData> plugins;
    if (parentApp.isEmpty()) {
        plugins = KPluginMetaData::findPlugins(QStringLiteral("lingmo/containmentactions"));
    } else {
        plugins = KPluginMetaData::findPlugins(QStringLiteral("lingmo/containmentactions"), filter);
    }

    return plugins;
}

} // Lingmo Namespace
