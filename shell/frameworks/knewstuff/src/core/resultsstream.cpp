/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "resultsstream.h"
#include "enginebase_p.h"
#include "knewstuffcore_debug.h"

#include <QTimer>

using namespace KNSCore;

class KNSCore::ResultsStreamPrivate
{
public:
    QList<QSharedPointer<KNSCore::Provider>> providers;
    EngineBase const *engine;
    Provider::SearchRequest request;
};

ResultsStream::ResultsStream(const Provider::SearchRequest &request, EngineBase *base)
    : d(new ResultsStreamPrivate{
        .providers = base->d->providers.values(),
        .engine = base,
        .request = request,
    })
{
    auto finished = [this](const KNSCore::Provider::SearchRequest &request, const KNSCore::Entry::List &entries) {
        if (request != d->request) {
            return;
        }

        if (d->providers.removeAll(qobject_cast<Provider *>(sender())) <= 0) {
            qCDebug(KNEWSTUFFCORE) << "Request finished twice, check your provider" << sender() << d->engine << entries.size();
            return;
        }

        if (entries.isEmpty() && d->providers.isEmpty()) {
            finish();
        }
        if (!entries.isEmpty()) {
            Q_EMIT entriesFound(entries);
        }
    };
    auto failed = [this](const KNSCore::Provider::SearchRequest &request) {
        if (request == d->request) {
            finish();
        }
    };
    for (const auto &provider : d->providers) {
        connect(provider.data(), &Provider::loadingFinished, this, finished);
        connect(provider.data(), &Provider::entryDetailsLoaded, this, [this](const KNSCore::Entry &entry) {
            if (d->request.filter == KNSCore::Provider::ExactEntryId && d->request.searchTerm == entry.uniqueId()) {
                if (entry.isValid()) {
                    Q_EMIT entriesFound({entry});
                }
                finish();
            }
        });
        connect(provider.data(), &Provider::loadingFailed, this, failed);
    }
}

ResultsStream::~ResultsStream() = default;

void ResultsStream::fetch()
{
    if (d->request.filter != Provider::Installed) {
        // when asking for installed entries, never use the cache
        Entry::List cacheEntries = d->engine->cache()->requestFromCache(d->request);
        if (!cacheEntries.isEmpty()) {
            Q_EMIT entriesFound(cacheEntries);
            return;
        }
    }

    for (const QSharedPointer<KNSCore::Provider> &p : std::as_const(d->providers)) {
        if (p->isInitialized()) {
            QTimer::singleShot(0, this, [this, p] {
                p->loadEntries(d->request);
            });
        } else {
            connect(p.get(), &KNSCore::Provider::providerInitialized, this, [this, p] {
                disconnect(p.get(), &KNSCore::Provider::providerInitialized, this, nullptr);
                p->loadEntries(d->request);
            });
        }
    }
}

void ResultsStream::fetchMore()
{
    d->request.page++;
    fetch();
}

void ResultsStream::finish()
{
    Q_EMIT finished();
    deleteLater();
}

#include "moc_resultsstream.cpp"
