/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "comic.h"

#include <QDate>
#include <QDebug>
#include <QFileInfo>
#include <QImage>
#include <QNetworkInformation>
#include <QSettings>
#include <QStandardPaths>

#include <KPackage/PackageLoader>
#include <qloggingcategory.h>

#include "cachedprovider.h"
#include "comic_debug.h"
#include "comicprovider.h"
#include "comicproviderkross.h"

ComicEngine::ComicEngine(QObject *parent)
    : QObject(parent)
    , mEmptySuffix(false)
{
    QNetworkInformation::instance()->loadBackendByFeatures(QNetworkInformation::Feature::Reachability);
    loadProviders();
}

QList<ComicProviderInfo> ComicEngine::loadProviders()
{
    mProviders.clear();
    const auto comics = KPackage::PackageLoader::self()->listKPackages(QStringLiteral("Plasma/Comic"));
    QList<ComicProviderInfo> providers;
    for (auto comicPackage : comics) {
        const KPluginMetaData comic = comicPackage.metadata();
        qCDebug(PLASMA_COMIC) << "ComicEngine::loadProviders()  service name=" << comic.name();
        ComicProviderInfo data;
        data.pluginId = comic.pluginId();
        data.name = comic.name();
        QFileInfo file(comic.iconName());
        if (file.isRelative()) {
            data.icon =
                QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString::fromLatin1("plasma/comics/%1/%2").arg(comic.pluginId(), comic.iconName()));
        } else {
            data.icon = comic.iconName();
        }
        mProviders << comic.pluginId();
        providers << data;
    }
    return providers;
}

void ComicEngine::setMaxComicLimit(int maxComicLimit)
{
    CachedProvider::setMaxComicLimit(maxComicLimit);
}

bool ComicEngine::requestSource(const QString &identifier)
{
    if (m_jobs.contains(identifier)) {
        return true;
    }

    const QStringList parts = identifier.split(QLatin1Char(':'), Qt::KeepEmptyParts);

    // check whether it is cached, make sure second part present
    if (parts.count() > 1 && (CachedProvider::isCached(identifier) || !isOnline())) {
        ComicProvider *provider = new CachedProvider(this, KPluginMetaData{}, IdentifierType::StringIdentifier, identifier);
        m_jobs[identifier] = provider;
        connect(provider, &ComicProvider::finished, this, &ComicEngine::finished);
        connect(provider, &ComicProvider::error, this, &ComicEngine::error);
        return true;
    }

    // ... start a new query otherwise
    if (parts.count() < 2) {
        Q_EMIT requestFinished(ComicMetaData{.error = true});
        qWarning() << "Less than two arguments specified.";
        return false;
    }
    if (!mProviders.contains(parts[0])) {
        // User might have installed more from GHNS
        loadProviders();
        if (!mProviders.contains(parts[0])) {
            Q_EMIT requestFinished(ComicMetaData{.error = true});
            qWarning() << identifier << "comic plugin does not seem to be installed.";
            return false;
        }
    }

    // check if there is a connection
    if (!isOnline()) {
        qCDebug(PLASMA_COMIC) << "Currently offline, requested identifier was" << mIdentifierError;
        mIdentifierError = identifier;
        ComicMetaData data;
        data.error = true;
        data.errorAutomaticallyFixable = true;
        data.identifier = identifier;
        data.previousIdentifier = lastCachedIdentifier(identifier);
        Q_EMIT requestFinished(data);
        qCDebug(PLASMA_COMIC) << "No internet connection, using cached data";
        return true;
    }

    KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Comic"), parts[0]);

    bool isCurrentComic = parts[1].isEmpty();

    ComicProvider *provider = nullptr;

    QVariant data;
    const IdentifierType identifierType = stringToIdentifierType(pkg.metadata().value(QStringLiteral("X-KDE-PlasmaComicProvider-SuffixType")));
    if (identifierType == IdentifierType::DateIdentifier) {
        QDate date = QDate::fromString(parts[1], Qt::ISODate);
        if (!date.isValid()) {
            date = QDate::currentDate();
        }

        data = date;
    } else if (identifierType == IdentifierType::NumberIdentifier) {
        data = parts[1].toInt();
    } else if (identifierType == IdentifierType::StringIdentifier) {
        data = parts[1];
    }
    provider = new ComicProviderKross(this, pkg.metadata(), identifierType, data);
    provider->setIsCurrent(isCurrentComic);

    m_jobs[identifier] = provider;

    connect(provider, &ComicProvider::finished, this, &ComicEngine::finished);
    connect(provider, &ComicProvider::error, this, &ComicEngine::error);
    return true;
}

void ComicEngine::finished(ComicProvider *provider)
{
    // sets the data
    if (provider->image().isNull()) {
        qCWarning(PLASMA_COMIC) << "Provider returned null image" << provider->name();
        error(provider);
        return;
    }

    ComicMetaData data = metaDataFromProvider(provider);

    // different comic -- with no error yet -- has been chosen, old error is invalidated
    QString temp = mIdentifierError.left(mIdentifierError.indexOf(QLatin1Char(':')) + 1);
    if (!mIdentifierError.isEmpty() && provider->identifier().indexOf(temp) == -1) {
        mIdentifierError.clear();
    }
    // comic strip with error worked now
    if (!mIdentifierError.isEmpty() && (mIdentifierError == provider->identifier())) {
        mIdentifierError.clear();
    }

    // store in cache if it's not the response of a CachedProvider,
    if (!provider->inherits("CachedProvider") && !provider->image().isNull()) {
        CachedProvider::storeInCache(provider->identifier(), provider->image(), data);
    }
    provider->deleteLater();

    const QString key = m_jobs.key(provider);
    if (!key.isEmpty()) {
        m_jobs.remove(key);
    }
    Q_EMIT requestFinished(data);
}

void ComicEngine::error(ComicProvider *provider)
{
    QString identifier(provider->identifier());
    mIdentifierError = identifier;

    qWarning() << identifier << "plugging reported an error.";

    ComicMetaData data = metaDataFromProvider(provider);
    data.error = true;

    // if there was an error loading the last cached comic strip, do not return its id anymore
    const QString lastCachedId = lastCachedIdentifier(identifier);
    if (lastCachedId != provider->identifier().mid(provider->identifier().indexOf(QLatin1Char(':')) + 1)) {
        // sets the previousIdentifier to the identifier of a strip that has been cached before
        data.previousIdentifier = lastCachedId;
    }
    data.nextIdentifier = QString();

    const QString key = m_jobs.key(provider);
    if (!key.isEmpty()) {
        m_jobs.remove(key);
    }

    provider->deleteLater();
    Q_EMIT requestFinished(data);
}

ComicMetaData ComicEngine::metaDataFromProvider(ComicProvider *provider)
{
    QString identifier(provider->identifier());

    /**
     * Requests for the current day have no suffix (date or id)
     * set initially, so we have to remove the 'faked' suffix
     * here again to not confuse the applet.
     */
    if (provider->isCurrent()) {
        identifier = identifier.left(identifier.indexOf(QLatin1Char(':')) + 1);
    }

    ComicMetaData data;
    data.imageUrl = provider->imageUrl();
    data.image = provider->image();
    data.websiteUrl = provider->websiteUrl();
    data.shopUrl = provider->shopUrl();
    data.nextIdentifier = provider->nextIdentifier();
    data.previousIdentifier = provider->previousIdentifier();
    data.comicAuthor = provider->comicAuthor();
    data.additionalText = provider->additionalText();
    data.stripTitle = provider->stripTitle();
    data.firstStripIdentifier = provider->firstStripIdentifier();
    data.identifier = identifier;
    data.providerName = provider->name();
    data.identifierType = provider->identifierType();
    data.isLeftToRight = provider->isLeftToRight();
    data.isTopToBottom = provider->isTopToBottom();

    return data;
}

QString ComicEngine::lastCachedIdentifier(const QString &identifier) const
{
    const QString id = identifier.left(identifier.indexOf(QLatin1Char(':')));
    QString data = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/plasma_engine_comic/");
    data += QString::fromLatin1(QUrl::toPercentEncoding(id));
    QSettings settings(data + QLatin1String(".conf"), QSettings::IniFormat);
    QString previousIdentifier = settings.value(QLatin1String("lastCachedStripIdentifier"), QString()).toString();

    return previousIdentifier;
}

bool ComicEngine::isOnline() const
{
    return QNetworkInformation::instance()->reachability() == QNetworkInformation::Reachability::Online;
}
