/*
    SPDX-FileCopyrightText: 2005-2009 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "knotifyconfig.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <QCache>
#include <QStandardPaths>

typedef QCache<QString, KSharedConfig::Ptr> ConfigCache;
Q_GLOBAL_STATIC_WITH_ARGS(ConfigCache, static_cache, (15))

class KNotifyConfigPrivate : public QSharedData
{
public:
    QString readEntry(const QString &group, const QString &key, bool path) const;

    QString applicationName;
    QString eventId;

    KSharedConfig::Ptr eventsFile;
    KSharedConfig::Ptr configFile;
};

QString KNotifyConfigPrivate::readEntry(const QString &group, const QString &key, bool path) const
{
    if (configFile->hasGroup(group)) {
        KConfigGroup cg(configFile, group);
        const QString value = path ? cg.readPathEntry(key, QString()) : cg.readEntry(key, QString());
        if (!value.isNull()) {
            return value;
        }
    }

    if (eventsFile->hasGroup(group)) {
        KConfigGroup cg(eventsFile, group);
        const QString value = path ? cg.readPathEntry(key, QString()) : cg.readEntry(key, QString());
        if (!value.isNull()) {
            return value;
        }
    }

    return QString();
}

static KSharedConfig::Ptr retrieve_from_cache(const QString &filename, QStandardPaths::StandardLocation type = QStandardPaths::GenericConfigLocation)
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    if (cache.contains(filename)) {
        return *cache[filename];
    }

    KSharedConfig::Ptr m = KSharedConfig::openConfig(filename, KConfig::NoGlobals, type);
    // also search for event config files in qrc resources
    if (type == QStandardPaths::GenericDataLocation) {
        m->addConfigSources({QStringLiteral(":/") + filename});
    }
    cache.insert(filename, new KSharedConfig::Ptr(m));

    return m;
}

void KNotifyConfig::reparseConfiguration()
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    const auto listFiles = cache.keys();
    for (const QString &filename : listFiles) {
        (*cache[filename])->reparseConfiguration();
    }
}

void KNotifyConfig::reparseSingleConfiguration(const QString &app)
{
    QCache<QString, KSharedConfig::Ptr> &cache = *static_cache;
    const QString appCacheKey = app + QStringLiteral(".notifyrc");
    if (cache.contains(appCacheKey)) {
        (*cache[appCacheKey])->reparseConfiguration();
    }
}

KNotifyConfig::KNotifyConfig(const QString &applicationName, const QString &eventId)
    : d(new KNotifyConfigPrivate)
{
    d->applicationName = applicationName;
    d->eventId = eventId;

    d->eventsFile = retrieve_from_cache(QLatin1String("knotifications6/") + applicationName + QLatin1String(".notifyrc"), QStandardPaths::GenericDataLocation);
    d->configFile = retrieve_from_cache(applicationName + QStringLiteral(".notifyrc"));
}

KNotifyConfig::KNotifyConfig(const KNotifyConfig &other)
    : d(other.d)
{
}

KNotifyConfig &KNotifyConfig::operator=(const KNotifyConfig &other)
{
    d = other.d;
    return *this;
}

KNotifyConfig::~KNotifyConfig() = default;

QString KNotifyConfig::applicationName() const
{
    return d->applicationName;
}

QString KNotifyConfig::eventId() const
{
    return d->eventId;
}

bool KNotifyConfig::isValid() const
{
    const QString group = QLatin1String("Event/") + d->eventId;
    return d->configFile->hasGroup(group) || d->eventsFile->hasGroup(group);
}

QString KNotifyConfig::readGlobalEntry(const QString &key) const
{
    return d->readEntry(QStringLiteral("Global"), key, false);
}

QString KNotifyConfig::readEntry(const QString &key) const
{
    const QString group = QLatin1String("Event/") + d->eventId;
    return d->readEntry(group, key, false);
}

QString KNotifyConfig::readPathEntry(const QString &key) const
{
    const QString group = QLatin1String("Event/") + d->eventId;
    return d->readEntry(group, key, true);
}
