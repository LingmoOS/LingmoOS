/*
    loader.cpp
    SPDX-FileCopyrightText: 2001, 2002, 2003 Frerich Raabe <raabe@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include "loader.h"
#include "dataretriever.h"
#include "documentsource.h"
#include "feed.h"
#include "global.h"
#include "loaderutil_p.h"
#include "parsercollection.h"

#include <QUrl>


#include <syndication_debug.h>

namespace Syndication
{
struct SYNDICATION_NO_EXPORT Loader::LoaderPrivate {
    LoaderPrivate()
    {
    }

    ~LoaderPrivate()
    {
        delete retriever;
    }

    DataRetriever *retriever = nullptr;
    Syndication::ErrorCode lastError = Success;
    int retrieverError = 0;
    QUrl discoveredFeedURL;
    QUrl url;
};

Loader *Loader::create()
{
    return new Loader;
}

Loader *Loader::create(QObject *object, const char *slot)
{
    Loader *loader = create();
    connect(loader, SIGNAL(loadingComplete(Syndication::Loader *, Syndication::FeedPtr, Syndication::ErrorCode)), object, slot);
    return loader;
}

Loader::Loader()
    : d(new LoaderPrivate)
{
}

Loader::~Loader() = default;

void Loader::loadFrom(const QUrl &url, DataRetriever *retriever)
{
    if (d->retriever != nullptr) {
        return;
    }

    d->url = url;
    d->retriever = retriever;

    connect(d->retriever, &DataRetriever::dataRetrieved, this, &Loader::slotRetrieverDone);

    d->retriever->retrieveData(url);
}

int Loader::retrieverError() const
{
    return d->retrieverError;
}

Syndication::ErrorCode Loader::errorCode() const
{
    return d->lastError;
}

void Loader::abort()
{
    if (d && d->retriever) {
        d->retriever->abort();
        delete d->retriever;
        d->retriever = nullptr;
    }

    Q_EMIT loadingComplete(this, FeedPtr(), Aborted);
    delete this;
}

QUrl Loader::discoveredFeedURL() const
{
    return d->discoveredFeedURL;
}

void Loader::slotRetrieverDone(const QByteArray &data, bool success)
{
    d->retrieverError = d->retriever->errorCode();
    ErrorCode status = Success;
    FeedPtr feed;
    delete d->retriever;
    d->retriever = nullptr;

    if (success) {
        DocumentSource src(data, d->url.url());
        feed = parserCollection()->parse(src);

        if (parserCollection()->lastError() != Syndication::Success) {
            status = parserCollection()->lastError();
            discoverFeeds(data);
        }
    } else {
        qCDebug(SYNDICATION_LOG) << "Retriever error:" << d->retrieverError;
        // retriever is a custom impl, so we set OtherRetrieverError
        status = OtherRetrieverError;
    }

    Q_EMIT loadingComplete(this, feed, status);

    delete this;
}

void Loader::discoverFeeds(const QByteArray &data)
{
    const QUrl url = LoaderUtil::parseFeed(data, d->url);
    if (!url.isEmpty()) {
        d->discoveredFeedURL = url;
    }
}

} // namespace Syndication

#include "moc_loader.cpp"
