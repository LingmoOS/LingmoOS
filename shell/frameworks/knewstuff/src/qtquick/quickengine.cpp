/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "quickengine.h"
#include "cache.h"
#include "errorcode.h"
#include "imageloader_p.h"
#include "installation_p.h"
#include "knewstuffquick_debug.h"
#include "quicksettings.h"

#include <KLocalizedString>
#include <QTimer>

#include "categoriesmodel.h"
#include "quickquestionlistener.h"
#include "searchpresetmodel.h"

class EnginePrivate
{
public:
    bool isValid = false;
    CategoriesModel *categoriesModel = nullptr;
    SearchPresetModel *searchPresetModel = nullptr;
    QString configFile;
    QTimer searchTimer;
    Engine::BusyState busyState;
    QString busyMessage;
    // the current request from providers
    KNSCore::Provider::SearchRequest currentRequest;
    KNSCore::Provider::SearchRequest storedRequest;
    // the page that is currently displayed, so it is not requested repeatedly
    int currentPage = -1;

    // when requesting entries from a provider, how many to ask for
    int pageSize = 20;

    int numDataJobs = 0;
    int numPictureJobs = 0;
    int numInstallJobs = 0;
};

Engine::Engine(QObject *parent)
    : KNSCore::EngineBase(parent)
    , d(new EnginePrivate)
{
    const auto setBusy = [this](Engine::BusyState state, const QString &msg) {
        setBusyState(state);
        d->busyMessage = msg;
    };
    setBusy(BusyOperation::Initializing, i18n("Loading data")); // For the user this should be the same as initializing

    KNewStuffQuick::QuickQuestionListener::instance();
    d->categoriesModel = new CategoriesModel(this);
    connect(d->categoriesModel, &QAbstractListModel::modelReset, this, &Engine::categoriesChanged);
    d->searchPresetModel = new SearchPresetModel(this);
    connect(d->searchPresetModel, &QAbstractListModel::modelReset, this, &Engine::searchPresetModelChanged);

    d->searchTimer.setSingleShot(true);
    d->searchTimer.setInterval(1000);
    connect(&d->searchTimer, &QTimer::timeout, this, &Engine::reloadEntries);
    connect(installation(), &KNSCore::Installation::signalInstallationFinished, this, [this]() {
        --d->numInstallJobs;
        updateStatus();
    });
    connect(installation(), &KNSCore::Installation::signalInstallationFailed, this, [this](const QString &message) {
        --d->numInstallJobs;
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::InstallationError, message, QVariant());
    });
    connect(this, &EngineBase::signalProvidersLoaded, this, &Engine::updateStatus);
    connect(this, &EngineBase::signalProvidersLoaded, this, [this]() {
        d->currentRequest.categories = EngineBase::categories();
    });

    connect(this,
            &KNSCore::EngineBase::signalErrorCode,
            this,
            [setBusy, this](const KNSCore::ErrorCode::ErrorCode &error, const QString &message, const QVariant &metadata) {
                Q_EMIT errorCode(error, message, metadata);
                if (error == KNSCore::ErrorCode::ProviderError || error == KNSCore::ErrorCode::ConfigFileError) {
                    // This means loading the config or providers file failed entirely and we cannot complete the
                    // initialisation. It also means the engine is done loading, but that nothing will
                    // work, and we need to inform the user of this.
                    setBusy({}, QString());
                }

                // Emit the signal later, currently QML is not connected to the slot
                if (error == KNSCore::ErrorCode::ConfigFileError) {
                    QTimer::singleShot(0, [this, error, message, metadata]() {
                        Q_EMIT errorCode(error, message, metadata);
                    });
                }
            });

    connect(this, &Engine::signalEntryEvent, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
        // Just forward the event but not do anything more
        Q_EMIT entryEvent(entry, event);
    });
    //
    // And finally, let's just make sure we don't miss out the various things here getting changed
    // In other words, when we're asked to reset the view, actually do that
    connect(this, &Engine::signalResetView, this, &Engine::categoriesFilterChanged);
    connect(this, &Engine::signalResetView, this, &Engine::filterChanged);
    connect(this, &Engine::signalResetView, this, &Engine::sortOrderChanged);
    connect(this, &Engine::signalResetView, this, &Engine::searchTermChanged);
}

bool Engine::init(const QString &configfile)
{
    const bool valid = EngineBase::init(configfile);
    if (valid) {
        connect(this, &Engine::signalEntryEvent, cache().data(), [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
            if (event == KNSCore::Entry::StatusChangedEvent) {
                cache()->registerChangedEntry(entry);
            }
        });
        const auto slotEntryChanged = [this](const KNSCore::Entry &entry) {
            Q_EMIT signalEntryEvent(entry, KNSCore::Entry::StatusChangedEvent);
        };
        connect(installation(), &KNSCore::Installation::signalEntryChanged, this, slotEntryChanged);
        connect(cache().data(), &KNSCore::Cache::entryChanged, this, slotEntryChanged);
    }
    return valid;
}
void Engine::updateStatus()
{
    QString busyMessage;
    BusyState state;
    if (d->numPictureJobs > 0) {
        // If it is loading previews or data is irrelevant for the user
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingPreview;
    }
    if (d->numInstallJobs > 0) {
        busyMessage = i18n("Installing");
        state |= BusyOperation::InstallingEntry;
    }
    if (d->numDataJobs > 0) {
        busyMessage = i18n("Loading data");
        state |= BusyOperation::LoadingData;
    }
    d->busyMessage = busyMessage;
    setBusyState(state);
}

bool Engine::needsLazyLoadSpinner()
{
    return d->numDataJobs > 0 || d->numPictureJobs;
}

Engine::~Engine() = default;

void Engine::setBusyState(BusyState state)
{
    d->busyState = state;
    Q_EMIT busyStateChanged();
}
Engine::BusyState Engine::busyState() const
{
    return d->busyState;
}
QString Engine::busyMessage() const
{
    return d->busyMessage;
}

QString Engine::configFile() const
{
    return d->configFile;
}

void Engine::setConfigFile(const QString &newFile)
{
    if (d->configFile != newFile) {
        d->configFile = newFile;
        Q_EMIT configFileChanged();

        if (KNewStuffQuick::Settings::instance()->allowedByKiosk()) {
            d->isValid = init(newFile);
            Q_EMIT categoriesFilterChanged();
            Q_EMIT filterChanged();
            Q_EMIT sortOrderChanged();
            Q_EMIT searchTermChanged();
        } else {
            // This is not an error message in the proper sense, and the message is not intended to look like an error (as there is really
            // nothing the user can do to fix it, and we just tell them so they're not wondering what's wrong)
            Q_EMIT errorCode(
                KNSCore::ErrorCode::ConfigFileError,
                i18nc("An informational message which is shown to inform the user they are not authorized to use GetHotNewStuff functionality",
                      "You are not authorized to Get Hot New Stuff. If you think this is in error, please contact the person in charge of your permissions."),
                QVariant());
        }
    }
}

CategoriesModel *Engine::categories() const
{
    return d->categoriesModel;
}

QStringList Engine::categoriesFilter() const
{
    return d->currentRequest.categories;
}

void Engine::setCategoriesFilter(const QStringList &newCategoriesFilter)
{
    if (d->currentRequest.categories != newCategoriesFilter) {
        d->currentRequest.categories = newCategoriesFilter;
        reloadEntries();
        Q_EMIT categoriesFilterChanged();
    }
}

KNSCore::Provider::Filter Engine::filter() const
{
    return d->currentRequest.filter;
}

void Engine::setFilter(KNSCore::Provider::Filter newFilter)
{
    if (d->currentRequest.filter != newFilter) {
        d->currentRequest.filter = newFilter;
        reloadEntries();
        Q_EMIT filterChanged();
    }
}

KNSCore::Provider::SortMode Engine::sortOrder() const
{
    return d->currentRequest.sortMode;
}

void Engine::setSortOrder(KNSCore::Provider::SortMode mode)
{
    if (d->currentRequest.sortMode != mode) {
        d->currentRequest.sortMode = mode;
        reloadEntries();
        Q_EMIT sortOrderChanged();
    }
}

QString Engine::searchTerm() const
{
    return d->currentRequest.searchTerm;
}

void Engine::setSearchTerm(const QString &searchTerm)
{
    if (d->isValid && d->currentRequest.searchTerm != searchTerm) {
        d->currentRequest.searchTerm = searchTerm;
        Q_EMIT searchTermChanged();
    }
    KNSCore::Entry::List cacheEntries = cache()->requestFromCache(d->currentRequest);
    if (!cacheEntries.isEmpty()) {
        reloadEntries();
    } else {
        d->searchTimer.start();
    }
}

SearchPresetModel *Engine::searchPresetModel() const
{
    return d->searchPresetModel;
}

bool Engine::isValid()
{
    return d->isValid;
}

void Engine::updateEntryContents(const KNSCore::Entry &entry)
{
    const auto provider = EngineBase::provider(entry.providerId());
    if (provider.isNull() || !provider->isInitialized()) {
        qCWarning(KNEWSTUFFQUICK) << "Provider was not found or is not initialized" << provider << entry.providerId();
        return;
    }
    provider->loadEntryDetails(entry);
}

void Engine::reloadEntries()
{
    Q_EMIT signalResetView();
    d->currentPage = -1;
    d->currentRequest.page = 0;
    d->numDataJobs = 0;

    const auto providersList = EngineBase::providers();
    for (const QSharedPointer<KNSCore::Provider> &p : providersList) {
        if (p->isInitialized()) {
            if (d->currentRequest.filter == KNSCore::Provider::Installed || d->currentRequest.filter == KNSCore::Provider::Updates) {
                // when asking for installed entries, never use the cache
                p->loadEntries(d->currentRequest);
            } else {
                // take entries from cache until there are no more
                KNSCore::Entry::List cacheEntries;
                KNSCore::Entry::List lastCache = cache()->requestFromCache(d->currentRequest);
                while (!lastCache.isEmpty()) {
                    qCDebug(KNEWSTUFFQUICK) << "From cache";
                    cacheEntries << lastCache;

                    d->currentPage = d->currentRequest.page;
                    ++d->currentRequest.page;
                    lastCache = cache()->requestFromCache(d->currentRequest);
                }

                // Since the cache has no more pages, reset the request's page
                if (d->currentPage >= 0) {
                    d->currentRequest.page = d->currentPage;
                }

                if (!cacheEntries.isEmpty()) {
                    Q_EMIT signalEntriesLoaded(cacheEntries);
                } else {
                    qCDebug(KNEWSTUFFQUICK) << "From provider";
                    p->loadEntries(d->currentRequest);

                    ++d->numDataJobs;
                    updateStatus();
                }
            }
        }
    }
}
void Engine::addProvider(QSharedPointer<KNSCore::Provider> provider)
{
    EngineBase::addProvider(provider);
    connect(provider.data(), &KNSCore::Provider::loadingFinished, this, [this](const auto &request, const auto &entries) {
        d->currentPage = qMax<int>(request.page, d->currentPage);
        qCDebug(KNEWSTUFFQUICK) << "loaded page " << request.page << "current page" << d->currentPage << "count:" << entries.count();

        if (request.filter != KNSCore::Provider::Updates) {
            cache()->insertRequest(request, entries);
        }
        Q_EMIT signalEntriesLoaded(entries);

        --d->numDataJobs;
        updateStatus();
    });
    connect(provider.data(), &KNSCore::Provider::entryDetailsLoaded, this, [this](const auto &entry) {
        --d->numDataJobs;
        updateStatus();
        Q_EMIT signalEntryEvent(entry, KNSCore::Entry::DetailsLoadedEvent);
    });
}

void Engine::loadPreview(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type)
{
    qCDebug(KNEWSTUFFQUICK) << "START  preview: " << entry.name() << type;
    auto l = new KNSCore::ImageLoader(entry, type, this);
    connect(l, &KNSCore::ImageLoader::signalPreviewLoaded, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type) {
        qCDebug(KNEWSTUFFQUICK) << "FINISH preview: " << entry.name() << type;
        Q_EMIT signalEntryPreviewLoaded(entry, type);
        --d->numPictureJobs;
        updateStatus();
    });
    connect(l, &KNSCore::ImageLoader::signalError, this, [this](const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type, const QString &errorText) {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ImageError, errorText, QVariantList() << entry.name() << type);
        qCDebug(KNEWSTUFFQUICK) << "ERROR preview: " << errorText << entry.name() << type;
        --d->numPictureJobs;
        updateStatus();
    });
    l->start();
    ++d->numPictureJobs;
    updateStatus();
}

void Engine::adoptEntry(const KNSCore::Entry &entry)
{
    registerTransaction(KNSCore::Transaction::adopt(this, entry));
}
void Engine::install(const KNSCore::Entry &entry, int linkId)
{
    auto transaction = KNSCore::Transaction::install(this, entry, linkId);
    registerTransaction(transaction);
    if (!transaction->isFinished()) {
        ++d->numInstallJobs;
    }
}
void Engine::uninstall(const KNSCore::Entry &entry)
{
    registerTransaction(KNSCore::Transaction::uninstall(this, entry));
}
void Engine::registerTransaction(KNSCore::Transaction *transaction)
{
    connect(transaction, &KNSCore::Transaction::signalErrorCode, this, &EngineBase::signalErrorCode);
    connect(transaction, &KNSCore::Transaction::signalMessage, this, &EngineBase::signalMessage);
    connect(transaction, &KNSCore::Transaction::signalEntryEvent, this, &Engine::signalEntryEvent);
}

void Engine::requestMoreData()
{
    qCDebug(KNEWSTUFFQUICK) << "Get more data! current page: " << d->currentPage << " requested: " << d->currentRequest.page;

    if (d->currentPage < d->currentRequest.page) {
        return;
    }

    d->currentRequest.page++;
    doRequest();
}
void Engine::doRequest()
{
    const auto providersList = providers();
    for (const QSharedPointer<KNSCore::Provider> &p : providersList) {
        if (p->isInitialized()) {
            p->loadEntries(d->currentRequest);
            ++d->numDataJobs;
            updateStatus();
        }
    }
}

void Engine::revalidateCacheEntries()
{
    // This gets called from QML, because in QtQuick we reuse the engine, BUG: 417985
    // We can't handle this in the cache, because it can't access the configuration of the engine
    if (cache()) {
        const auto providersList = providers();
        for (const auto &provider : providersList) {
            if (provider && provider->isInitialized()) {
                const KNSCore::Entry::List cacheBefore = cache()->registryForProvider(provider->id());
                cache()->removeDeletedEntries();
                const KNSCore::Entry::List cacheAfter = cache()->registryForProvider(provider->id());
                // If the user has deleted them in the background we have to update the state to deleted
                for (const auto &oldCachedEntry : cacheBefore) {
                    if (!cacheAfter.contains(oldCachedEntry)) {
                        KNSCore::Entry removedEntry = oldCachedEntry;
                        removedEntry.setEntryDeleted();
                        Q_EMIT signalEntryEvent(removedEntry, KNSCore::Entry::StatusChangedEvent);
                    }
                }
            }
        }
    }
}

void Engine::restoreSearch()
{
    d->searchTimer.stop();
    d->currentRequest = d->storedRequest;
    if (cache()) {
        KNSCore::Entry::List cacheEntries = cache()->requestFromCache(d->currentRequest);
        if (!cacheEntries.isEmpty()) {
            reloadEntries();
        } else {
            d->searchTimer.start();
        }
    } else {
        qCWarning(KNEWSTUFFQUICK) << "Attempted to call restoreSearch() without a correctly initialized engine. You will likely get unexpected behaviour.";
    }
}

void Engine::storeSearch()
{
    d->storedRequest = d->currentRequest;
}

#include "moc_quickengine.cpp"
