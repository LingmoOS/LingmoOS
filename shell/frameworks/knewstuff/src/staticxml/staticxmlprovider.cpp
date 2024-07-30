/*
    knewstuff3/provider.cpp
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "staticxmlprovider_p.h"

#include "xmlloader_p.h"

#include <QTimer>
#include <knewstuffcore_debug.h>
#include <tagsfilterchecker.h>

namespace KNSCore
{
StaticXmlProvider::StaticXmlProvider()
    : mInitialized(false)
{
}

QString StaticXmlProvider::id() const
{
    return mId;
}

bool StaticXmlProvider::setProviderXML(const QDomElement &xmldata)
{
    if (xmldata.tagName() != QLatin1String("provider")) {
        return false;
    }

    mUploadUrl = QUrl(xmldata.attribute(QStringLiteral("uploadurl")));
    mNoUploadUrl = QUrl(xmldata.attribute(QStringLiteral("nouploadurl")));

    QString url = xmldata.attribute(QStringLiteral("downloadurl"));
    if (!url.isEmpty()) {
        mDownloadUrls.insert(QString(), QUrl(url));
    }

    url = xmldata.attribute(QStringLiteral("downloadurl-latest"));
    if (!url.isEmpty()) {
        mDownloadUrls.insert(QStringLiteral("latest"), QUrl(url));
    }

    url = xmldata.attribute(QStringLiteral("downloadurl-score"));
    if (!url.isEmpty()) {
        mDownloadUrls.insert(QStringLiteral("score"), QUrl(url));
    }

    url = xmldata.attribute(QStringLiteral("downloadurl-downloads"));
    if (!url.isEmpty()) {
        mDownloadUrls.insert(QStringLiteral("downloads"), QUrl(url));
    }

    // FIXME: this depends on freedesktop.org icon naming... introduce 'desktopicon'?
    QUrl iconurl(xmldata.attribute(QStringLiteral("icon")));
    if (!iconurl.isValid()) {
        iconurl = QUrl::fromLocalFile(xmldata.attribute(QStringLiteral("icon")));
    }
    setIcon(iconurl);

    QDomNode n;
    QLocale::Language systemLanguage = QLocale::system().language();
    QString firstName;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == QLatin1String("title")) {
            const QString lang{e.attribute(QLatin1String("lang"))};
            bool useThisTitle{false};
            if (name().isEmpty() && lang.isEmpty()) {
                // If we have no title as yet, and we've also got no language defined, this is the default
                // and name we need to set it, even if we might override it later
                useThisTitle = true;
            } else {
                const QLocale locale(lang);
                if (systemLanguage == locale.language()) {
                    useThisTitle = true;
                }
            }
            if (useThisTitle) {
                setName(e.text().trimmed());
                qCDebug(KNEWSTUFFCORE) << "add name for provider (" << this << "): " << e.text();
            }
            if (firstName.isEmpty()) {
                firstName = e.text().trimmed();
            }
        }
    }
    if (name().isEmpty()) {
        // Just a fallback, because those are quite nice to have...
        setName(firstName);
    }

    // Validation
    if ((mNoUploadUrl.isValid()) && (mUploadUrl.isValid())) {
        qWarning() << "StaticXmlProvider: both uploadurl and nouploadurl given";
        return false;
    }

    if ((!mNoUploadUrl.isValid()) && (!mUploadUrl.isValid())) {
        qWarning() << "StaticXmlProvider: neither uploadurl nor nouploadurl given";
        return false;
    }

    if (mUploadUrl.isValid()) {
        setWebsite(mUploadUrl);
    } else {
        setWebsite(mNoUploadUrl);
    }

    mId = mDownloadUrls[QString()].url();
    if (mId.isEmpty()) {
        mId = mDownloadUrls[mDownloadUrls.begin().key()].url();
    }

    QTimer::singleShot(0, this, &StaticXmlProvider::slotEmitProviderInitialized);

    return true;
}

void StaticXmlProvider::slotEmitProviderInitialized()
{
    mInitialized = true;
    Q_EMIT providerInitialized(this);
}

bool StaticXmlProvider::isInitialized() const
{
    return mInitialized;
}

void StaticXmlProvider::setCachedEntries(const KNSCore::Entry::List &cachedEntries)
{
    qCDebug(KNEWSTUFFCORE) << "Set cached entries " << cachedEntries.size();
    mCachedEntries.append(cachedEntries);
}

void StaticXmlProvider::loadEntries(const KNSCore::Provider::SearchRequest &request)
{
    mCurrentRequest = request;

    // static providers only have on page containing everything
    if (request.page > 0) {
        Q_EMIT loadingFinished(request, Entry::List());
        return;
    }

    if (request.filter == Installed) {
        qCDebug(KNEWSTUFFCORE) << "Installed entries: " << mId << installedEntries().size();
        if (request.page == 0) {
            Q_EMIT loadingFinished(request, installedEntries());
        } else {
            Q_EMIT loadingFinished(request, Entry::List());
        }
        return;
    }

    QUrl url = downloadUrl(request.sortMode);
    if (!url.isEmpty()) {
        // TODO first get the entries, then filter with searchString, finally emit the finished signal...
        // FIXME: don't create an endless number of xmlloaders!
        XmlLoader *loader = new XmlLoader(this);
        connect(loader, &XmlLoader::signalLoaded, this, &StaticXmlProvider::slotFeedFileLoaded);
        connect(loader, &XmlLoader::signalFailed, this, &StaticXmlProvider::slotFeedFailed);
        loader->setFilter(request.filter);
        loader->setSearchTerm(request.searchTerm);

        mFeedLoaders.insert(request.sortMode, loader);

        loader->load(url);
    } else {
        Q_EMIT loadingFailed(request);
    }
}

QUrl StaticXmlProvider::downloadUrl(SortMode mode) const
{
    QUrl url;
    switch (mode) {
    case Rating:
        url = mDownloadUrls.value(QStringLiteral("score"));
        break;
    case Alphabetical:
        url = mDownloadUrls.value(QString());
        break;
    case Newest:
        url = mDownloadUrls.value(QStringLiteral("latest"));
        break;
    case Downloads:
        url = mDownloadUrls.value(QStringLiteral("downloads"));
        break;
    }
    if (url.isEmpty()) {
        url = mDownloadUrls.value(QString());
    }
    return url;
}

void StaticXmlProvider::slotFeedFileLoaded(const QDomDocument &doc)
{
    XmlLoader *loader = qobject_cast<KNSCore::XmlLoader *>(sender());
    if (!loader) {
        qWarning() << "Loader not found!";
        Q_EMIT loadingFailed(mCurrentRequest);
        return;
    }

    // load all the entries from the domdocument given
    Entry::List entries;
    QDomElement element;

    TagsFilterChecker checker(tagFilter());
    TagsFilterChecker downloadschecker(downloadTagFilter());
    element = doc.documentElement();
    QDomElement n;
    for (n = element.firstChildElement(); !n.isNull(); n = n.nextSiblingElement()) {
        Entry entry;
        entry.setEntryXML(n.toElement());
        entry.setStatus(KNSCore::Entry::Downloadable);
        entry.setProviderId(mId);

        int index = mCachedEntries.indexOf(entry);
        if (index >= 0) {
            Entry cacheEntry = mCachedEntries.takeAt(index);
            // check if updateable
            if ((cacheEntry.status() == KNSCore::Entry::Installed)
                && ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
                entry.setStatus(KNSCore::Entry::Updateable);
                entry.setUpdateVersion(entry.version());
                entry.setVersion(cacheEntry.version());
                entry.setUpdateReleaseDate(entry.releaseDate());
                entry.setReleaseDate(cacheEntry.releaseDate());
            } else {
                entry.setStatus(cacheEntry.status());
            }
            cacheEntry = entry;
        }

        if (checker.filterAccepts(entry.tags())) {
            bool filterAcceptsDownloads = true;
            if (entry.downloadCount() > 0) {
                const auto downloadInfoList = entry.downloadLinkInformationList();
                for (const KNSCore::Entry::DownloadLinkInformation &dli : downloadInfoList) {
                    if (downloadschecker.filterAccepts(dli.tags)) {
                        filterAcceptsDownloads = true;
                        break;
                    }
                }
            }
            if (filterAcceptsDownloads) {
                mCachedEntries.append(entry);

                if (searchIncludesEntry(entry)) {
                    switch (loader->filter()) {
                    case Installed:
                        // This is dealth with in loadEntries separately
                        Q_UNREACHABLE();
                    case Updates:
                        if (entry.status() == KNSCore::Entry::Updateable) {
                            entries << entry;
                        }
                        break;
                    case ExactEntryId:
                        if (entry.uniqueId() == loader->searchTerm()) {
                            entries << entry;
                        }
                        break;
                    case None:
                        entries << entry;
                        break;
                    }
                }
            } else {
                qCDebug(KNEWSTUFFCORE) << "Filter has excluded" << entry.name() << "on download filter" << downloadTagFilter();
            }
        } else {
            qCDebug(KNEWSTUFFCORE) << "Filter has excluded" << entry.name() << "on entry filter" << tagFilter();
        }
    }
    Q_EMIT loadingFinished(mCurrentRequest, entries);
}

void StaticXmlProvider::slotFeedFailed()
{
    Q_EMIT loadingFailed(mCurrentRequest);
}

bool StaticXmlProvider::searchIncludesEntry(const KNSCore::Entry &entry) const
{
    if (mCurrentRequest.filter == Updates) {
        if (entry.status() != KNSCore::Entry::Updateable) {
            return false;
        }
    }

    if (mCurrentRequest.searchTerm.isEmpty()) {
        return true;
    }
    QString search = mCurrentRequest.searchTerm;
    if (entry.name().contains(search, Qt::CaseInsensitive) || entry.summary().contains(search, Qt::CaseInsensitive)
        || entry.author().name().contains(search, Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

void StaticXmlProvider::loadPayloadLink(const KNSCore::Entry &entry, int)
{
    qCDebug(KNEWSTUFFCORE) << "Payload: " << entry.payload();
    Q_EMIT payloadLinkLoaded(entry);
}

Entry::List StaticXmlProvider::installedEntries() const
{
    Entry::List entries;
    for (const Entry &entry : std::as_const(mCachedEntries)) {
        if (entry.status() == KNSCore::Entry::Installed || entry.status() == KNSCore::Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

}

#include "moc_staticxmlprovider_p.cpp"
