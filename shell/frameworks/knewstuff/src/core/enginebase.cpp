/*
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "enginebase.h"
#include "enginebase_p.h"
#include <knewstuffcore_debug.h>

#include <KConfig>
#include <KConfigGroup>
#include <KFileUtils>
#include <KFormat>
#include <KLocalizedString>

#include <QFileInfo>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QThreadStorage>
#include <QTimer>

#include "attica/atticaprovider_p.h"
#include "opds/opdsprovider_p.h"
#include "resultsstream.h"
#include "staticxml/staticxmlprovider_p.h"
#include "transaction.h"
#include "xmlloader_p.h"

using namespace KNSCore;

typedef QHash<QUrl, QPointer<XmlLoader>> EngineProviderLoaderHash;
Q_GLOBAL_STATIC(QThreadStorage<EngineProviderLoaderHash>, s_engineProviderLoaders)

EngineBase::EngineBase(QObject *parent)
    : QObject(parent)
    , d(new EngineBasePrivate)
{
    connect(d->installation, &Installation::signalInstallationError, this, [this](const QString &message) {
        Q_EMIT signalErrorCode(ErrorCode::InstallationError, i18n("An error occurred during the installation process:\n%1", message), QVariant());
    });
}

QStringList EngineBase::availableConfigFiles()
{
    QStringList configSearchLocations;
    configSearchLocations << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, //
                                                       QStringLiteral("knsrcfiles"),
                                                       QStandardPaths::LocateDirectory);
    configSearchLocations << QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    return KFileUtils::findAllUniqueFiles(configSearchLocations, {QStringLiteral("*.knsrc")});
}

EngineBase::~EngineBase()
{
    if (d->cache) {
        d->cache->writeRegistry();
    }
    delete d->atticaProviderManager;
    delete d->installation;
}

bool EngineBase::init(const QString &configfile)
{
    qCDebug(KNEWSTUFFCORE) << "Initializing KNSCore::EngineBase from" << configfile;

    QString resolvedConfigFilePath;
    if (QFileInfo(configfile).isAbsolute()) {
        resolvedConfigFilePath = configfile; // It is an absolute path
    } else {
        // Don't do the expensive search unless the config is relative
        resolvedConfigFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("knsrcfiles/%1").arg(configfile));
    }

    if (!QFileInfo::exists(resolvedConfigFilePath)) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("Configuration file does not exist: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << "The knsrc file" << configfile << "does not exist";
        return false;
    }

    const KConfig conf(resolvedConfigFilePath);

    if (conf.accessMode() == KConfig::NoAccess) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("Configuration file exists, but cannot be opened: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << "The knsrc file" << configfile << "was found but could not be opened.";
        return false;
    }

    const KConfigGroup group = conf.hasGroup(QStringLiteral("KNewStuff")) ? conf.group(QStringLiteral("KNewStuff")) : conf.group(QStringLiteral("KNewStuff3"));
    if (!group.exists()) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("Configuration file is invalid: \"%1\"", configfile), configfile);
        qCCritical(KNEWSTUFFCORE) << configfile << "doesn't contain a KNewStuff or KNewStuff3 section.";
        return false;
    }

    d->name = group.readEntry("Name");
    d->categories = group.readEntry("Categories", QStringList());
    qCDebug(KNEWSTUFFCORE) << "Categories: " << d->categories;
    d->adoptionCommand = group.readEntry("AdoptionCommand");
    d->useLabel = group.readEntry("UseLabel", i18n("Use"));
    Q_EMIT useLabelChanged();
    d->uploadEnabled = group.readEntry("UploadEnabled", true);
    Q_EMIT uploadEnabledChanged();

    d->providerFileUrl = group.readEntry("ProvidersUrl", QUrl(QStringLiteral("https://autoconfig.kde.org/ocs/providers.xml")));
    if (group.readEntry("UseLocalProvidersFile", false)) {
        // The local providers file is called "appname.providers", to match "appname.knsrc"
        d->providerFileUrl = QUrl::fromLocalFile(QLatin1String("%1.providers").arg(configfile.left(configfile.length() - 6)));
    }

    d->tagFilter = group.readEntry("TagFilter", QStringList(QStringLiteral("ghns_excluded!=1")));
    d->downloadTagFilter = group.readEntry("DownloadTagFilter", QStringList());

    QByteArray rawContentWarningType = group.readEntry("ContentWarning", QByteArrayLiteral("Static"));
    bool ok = false;
    int value = QMetaEnum::fromType<ContentWarningType>().keyToValue(rawContentWarningType.constData(), &ok);
    if (ok) {
        d->contentWarningType = static_cast<ContentWarningType>(value);
    } else {
        qCWarning(KNEWSTUFFCORE) << "Could not parse ContentWarning, invalid entry" << rawContentWarningType;
    }

    Q_EMIT contentWarningTypeChanged();

    // Make sure that config is valid
    QString error;
    if (!d->installation->readConfig(group, error)) {
        Q_EMIT signalErrorCode(ErrorCode::ConfigFileError,
                               i18n("Could not initialise the installation handler for %1:\n%2\n"
                                    "This is a critical error and should be reported to the application author",
                                    configfile,
                                    error),
                               configfile);
        return false;
    }

    const QString configFileBasename = QFileInfo(resolvedConfigFilePath).completeBaseName();
    d->cache = Cache::getCache(configFileBasename);
    qCDebug(KNEWSTUFFCORE) << "Cache is" << d->cache << "for" << configFileBasename;
    d->cache->readRegistry();

    // Cache cleanup option, to help work around people deleting files from underneath KNewStuff (this
    // happens a lot with e.g. wallpapers and icons)
    if (d->installation->uncompressionSetting() == Installation::UseKPackageUncompression) {
        d->shouldRemoveDeletedEntries = true;
    }

    d->shouldRemoveDeletedEntries = group.readEntry("RemoveDeadEntries", d->shouldRemoveDeletedEntries);
    if (d->shouldRemoveDeletedEntries) {
        d->cache->removeDeletedEntries();
    }

    loadProviders();

    return true;
}

void EngineBase::loadProviders()
{
    if (d->providerFileUrl.isEmpty()) {
        // it would be nicer to move the attica stuff into its own class
        qCDebug(KNEWSTUFFCORE) << "Using OCS default providers";
        delete d->atticaProviderManager;
        d->atticaProviderManager = new Attica::ProviderManager;
        connect(d->atticaProviderManager, &Attica::ProviderManager::providerAdded, this, &EngineBase::atticaProviderLoaded);
        connect(d->atticaProviderManager, &Attica::ProviderManager::failedToLoad, this, &EngineBase::slotProvidersFailed);
        d->atticaProviderManager->loadDefaultProviders();
    } else {
        qCDebug(KNEWSTUFFCORE) << "loading providers from " << d->providerFileUrl;
        Q_EMIT loadingProvider();

        XmlLoader *loader = s_engineProviderLoaders()->localData().value(d->providerFileUrl);
        if (!loader) {
            qCDebug(KNEWSTUFFCORE) << "No xml loader for this url yet, so create one and temporarily store that" << d->providerFileUrl;
            loader = new XmlLoader(this);
            s_engineProviderLoaders()->localData().insert(d->providerFileUrl, loader);
            connect(loader, &XmlLoader::signalLoaded, this, [this]() {
                s_engineProviderLoaders()->localData().remove(d->providerFileUrl);
            });
            connect(loader, &XmlLoader::signalFailed, this, [this]() {
                s_engineProviderLoaders()->localData().remove(d->providerFileUrl);
            });
            connect(loader, &XmlLoader::signalHttpError, this, [this](int status, QList<QNetworkReply::RawHeaderPair> rawHeaders) {
                if (status == 503) { // Temporarily Unavailable
                    QDateTime retryAfter;
                    static const QByteArray retryAfterKey{"Retry-After"};
                    for (const QNetworkReply::RawHeaderPair &headerPair : rawHeaders) {
                        if (headerPair.first == retryAfterKey) {
                            // Retry-After is not a known header, so we need to do a bit of running around to make that work
                            // Also, the fromHttpDate function is in the private qnetworkrequest header, so we can't use that
                            // So, simple workaround, just pass it through a dummy request and get a formatted date out (the
                            // cost is sufficiently low here, given we've just done a bunch of i/o heavy things, so...)
                            QNetworkRequest dummyRequest;
                            dummyRequest.setRawHeader(QByteArray{"Last-Modified"}, headerPair.second);
                            retryAfter = dummyRequest.header(QNetworkRequest::LastModifiedHeader).toDateTime();
                            break;
                        }
                    }
                    QTimer::singleShot(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch(), this, &EngineBase::loadProviders);
                    // if it's a matter of a human moment's worth of seconds, just reload
                    if (retryAfter.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch() > 2) {
                        // more than that, spit out TryAgainLaterError to let the user know what we're doing with their time
                        static const KFormat formatter;
                        Q_EMIT signalErrorCode(KNSCore::ErrorCode::TryAgainLaterError,
                                               i18n("The service is currently undergoing maintenance and is expected to be back in %1.",
                                                    formatter.formatSpelloutDuration(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch())),
                                               {retryAfter});
                    }
                }
            });
            loader->load(d->providerFileUrl);
        }
        connect(loader, &XmlLoader::signalLoaded, this, &EngineBase::slotProviderFileLoaded);
        connect(loader, &XmlLoader::signalFailed, this, &EngineBase::slotProvidersFailed);
    }
}

QString KNSCore::EngineBase::name() const
{
    return d->name;
}

QStringList EngineBase::categories() const
{
    return d->categories;
}

QList<Provider::CategoryMetadata> EngineBase::categoriesMetadata()
{
    return d->categoriesMetadata;
}

QList<Provider::SearchPreset> EngineBase::searchPresets()
{
    return d->searchPresets;
}

QString EngineBase::useLabel() const
{
    return d->useLabel;
}

bool EngineBase::uploadEnabled() const
{
    return d->uploadEnabled;
}

void EngineBase::addProvider(QSharedPointer<KNSCore::Provider> provider)
{
    qCDebug(KNEWSTUFFCORE) << "Engine addProvider called with provider with id " << provider->id();
    d->providers.insert(provider->id(), provider);
    provider->setTagFilter(d->tagFilter);
    provider->setDownloadTagFilter(d->downloadTagFilter);
    connect(provider.data(), &Provider::providerInitialized, this, &EngineBase::providerInitialized);

    connect(provider.data(), &Provider::signalError, this, [this, provider](const QString &msg) {
        Q_EMIT signalErrorCode(ErrorCode::ProviderError, msg, d->providerFileUrl);
    });
    connect(provider.data(), &Provider::signalErrorCode, this, &EngineBase::signalErrorCode);
    connect(provider.data(), &Provider::signalInformation, this, &EngineBase::signalMessage);
    connect(provider.data(), &Provider::basicsLoaded, this, &EngineBase::providersChanged);
    Q_EMIT providersChanged();
}

void EngineBase::providerInitialized(Provider *p)
{
    qCDebug(KNEWSTUFFCORE) << "providerInitialized" << p->name();
    p->setCachedEntries(d->cache->registryForProvider(p->id()));

    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        if (!p->isInitialized()) {
            return;
        }
    }
    Q_EMIT signalProvidersLoaded();
}

void EngineBase::slotProvidersFailed()
{
    Q_EMIT signalErrorCode(KNSCore::ErrorCode::ProviderError,
                           i18n("Loading of providers from file: %1 failed", d->providerFileUrl.toString()),
                           d->providerFileUrl);
}

void EngineBase::slotProviderFileLoaded(const QDomDocument &doc)
{
    qCDebug(KNEWSTUFFCORE) << "slotProvidersLoaded";

    bool isAtticaProviderFile = false;

    // get each provider element, and create a provider object from it
    QDomElement providers = doc.documentElement();

    if (providers.tagName() == QLatin1String("providers")) {
        isAtticaProviderFile = true;
    } else if (providers.tagName() != QLatin1String("ghnsproviders") && providers.tagName() != QLatin1String("knewstuffproviders")) {
        qWarning() << "No document in providers.xml.";
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ProviderError,
                               i18n("Could not load get hot new stuff providers from file: %1", d->providerFileUrl.toString()),
                               d->providerFileUrl);
        return;
    }

    QDomElement n = providers.firstChildElement(QStringLiteral("provider"));
    while (!n.isNull()) {
        qCDebug(KNEWSTUFFCORE) << "Provider attributes: " << n.attribute(QStringLiteral("type"));

        QSharedPointer<KNSCore::Provider> provider;
        if (isAtticaProviderFile || n.attribute(QStringLiteral("type")).toLower() == QLatin1String("rest")) {
            provider.reset(new AtticaProvider(d->categories, {}));
            connect(provider.data(), &Provider::categoriesMetadataLoded, this, [this](const QList<Provider::CategoryMetadata> &categories) {
                d->categoriesMetadata = categories;
                Q_EMIT signalCategoriesMetadataLoded(categories);
            });
#ifdef SYNDICATION_FOUND
        } else if (n.attribute(QStringLiteral("type")).toLower() == QLatin1String("opds")) {
            provider.reset(new OPDSProvider);
            connect(provider.data(), &Provider::searchPresetsLoaded, this, [this](const QList<Provider::SearchPreset> &presets) {
                d->searchPresets = presets;
                Q_EMIT signalSearchPresetsLoaded(presets);
            });
#endif
        } else {
            provider.reset(new StaticXmlProvider);
        }

        if (provider->setProviderXML(n)) {
            addProvider(provider);
        } else {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::ProviderError, i18n("Error initializing provider."), d->providerFileUrl);
        }
        n = n.nextSiblingElement();
    }
    Q_EMIT loadingProvider();
}

void EngineBase::atticaProviderLoaded(const Attica::Provider &atticaProvider)
{
    qCDebug(KNEWSTUFFCORE) << "atticaProviderLoaded called";
    if (!atticaProvider.hasContentService()) {
        qCDebug(KNEWSTUFFCORE) << "Found provider: " << atticaProvider.baseUrl() << " but it does not support content";
        return;
    }
    QSharedPointer<KNSCore::Provider> provider = QSharedPointer<KNSCore::Provider>(new AtticaProvider(atticaProvider, d->categories, {}));
    connect(provider.data(), &Provider::categoriesMetadataLoded, this, [this](const QList<Provider::CategoryMetadata> &categories) {
        d->categoriesMetadata = categories;
        Q_EMIT signalCategoriesMetadataLoded(categories);
    });
    addProvider(provider);
}

QSharedPointer<Cache> EngineBase::cache() const
{
    return d->cache;
}

void EngineBase::setTagFilter(const QStringList &filter)
{
    d->tagFilter = filter;
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        p->setTagFilter(d->tagFilter);
    }
}

QStringList EngineBase::tagFilter() const
{
    return d->tagFilter;
}

void KNSCore::EngineBase::addTagFilter(const QString &filter)
{
    d->tagFilter << filter;
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        p->setTagFilter(d->tagFilter);
    }
}

void EngineBase::setDownloadTagFilter(const QStringList &filter)
{
    d->downloadTagFilter = filter;
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        p->setDownloadTagFilter(d->downloadTagFilter);
    }
}

QStringList EngineBase::downloadTagFilter() const
{
    return d->downloadTagFilter;
}

void EngineBase::addDownloadTagFilter(const QString &filter)
{
    d->downloadTagFilter << filter;
    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        p->setDownloadTagFilter(d->downloadTagFilter);
    }
}

QList<Attica::Provider *> EngineBase::atticaProviders() const
{
    QList<Attica::Provider *> ret;
    ret.reserve(d->providers.size());
    for (const auto &p : std::as_const(d->providers)) {
        const auto atticaProvider = p.dynamicCast<AtticaProvider>();
        if (atticaProvider) {
            ret += atticaProvider->provider();
        }
    }
    return ret;
}

bool EngineBase::userCanVote(const Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    return p->userCanVote();
}

void EngineBase::vote(const Entry &entry, uint rating)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    p->vote(entry, rating);
}

bool EngineBase::userCanBecomeFan(const Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    return p->userCanBecomeFan();
}

void EngineBase::becomeFan(const Entry &entry)
{
    QSharedPointer<Provider> p = d->providers.value(entry.providerId());
    p->becomeFan(entry);
}

QSharedPointer<Provider> EngineBase::provider(const QString &providerId) const
{
    return d->providers.value(providerId);
}

QSharedPointer<Provider> EngineBase::defaultProvider() const
{
    if (d->providers.count() > 0) {
        return d->providers.constBegin().value();
    }
    return nullptr;
}

QStringList EngineBase::providerIDs() const
{
    return d->providers.keys();
}

bool EngineBase::hasAdoptionCommand() const
{
    return !d->adoptionCommand.isEmpty();
}

void EngineBase::updateStatus()
{
}

Installation *EngineBase::installation() const
{
    return d->installation;
}

ResultsStream *EngineBase::search(const Provider::SearchRequest &request)
{
    return new ResultsStream(request, this);
}

EngineBase::ContentWarningType EngineBase::contentWarningType() const
{
    return d->contentWarningType;
}

QList<QSharedPointer<Provider>> EngineBase::providers() const
{
    return d->providers.values();
}

#include "moc_enginebase.cpp"
