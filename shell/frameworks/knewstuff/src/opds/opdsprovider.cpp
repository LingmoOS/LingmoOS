/*
    SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "opdsprovider_p.h"

#include <KFormat>
#include <KLocalizedString>
#include <QDate>
#include <QIcon>
#include <QLocale>
#include <QTimer>
#include <QUrlQuery>
#include <syndication/atom/atom.h>
#include <syndication/documentsource.h>

#include <knewstuffcore_debug.h>

#include "tagsfilterchecker.h"

namespace KNSCore
{
static const QLatin1String OPDS_REL_ACQUISITION{"http://opds-spec.org/acquisition"};
static const QLatin1String OPDS_REL_AC_OPEN_ACCESS{"http://opds-spec.org/acquisition/open-access"};
static const QLatin1String OPDS_REL_AC_BORROW{"http://opds-spec.org/acquisition/borrow"};
static const QLatin1String OPDS_REL_AC_BUY{"http://opds-spec.org/acquisition/buy"};
static const QLatin1String OPDS_REL_AC_SUBSCRIBE{"http://opds-spec.org/acquisition/subscribe"};
// static const QLatin1String OPDS_REL_AC_SAMPLE{"http://opds-spec.org/acquisition/sample"};
static const QLatin1String OPDS_REL_IMAGE{"http://opds-spec.org/image"};
static const QLatin1String OPDS_REL_THUMBNAIL{"http://opds-spec.org/image/thumbnail"};
static const QLatin1String OPDS_REL_CRAWL{"http://opds-spec.org/crawlable"};
// static const QLatin1String OPDS_REL_FACET{"http://opds-spec.org/facet"};
static const QLatin1String OPDS_REL_SHELF{"http://opds-spec.org/shelf"};
static const QLatin1String OPDS_REL_SORT_NEW{"http://opds-spec.org/sort/new"};
static const QLatin1String OPDS_REL_SORT_POPULAR{"http://opds-spec.org/sort/popular"};
static const QLatin1String OPDS_REL_FEATURED{"http://opds-spec.org/featured"};
static const QLatin1String OPDS_REL_RECOMMENDED{"http://opds-spec.org/recommended"};
static const QLatin1String OPDS_REL_SUBSCRIPTIONS{"http://opds-spec.org/subscriptions"};
static const QLatin1String OPDS_EL_PRICE{"opds:price"};
// static const QLatin1String OPDS_EL_INDIRECT{"opds:indirectAcquisition"};
// static const QLatin1String OPDS_ATTR_FACET_GROUP{"opds:facetGroup"};
// static const QLatin1String OPDS_ATTR_ACTIVE_FACET{"opds:activeFacet"};

static const QLatin1String OPDS_ATOM_MT{"application/atom+xml"};
static const QLatin1String OPDS_PROFILE{"profile=opds-catalog"};
static const QLatin1String OPDS_TYPE_ENTRY{"type=entry"};
// static const QLatin1String OPDS_KIND_NAVIGATION{"kind=navigation"};
// static const QLatin1String OPDS_KIND_ACQUISITION{"kind=acquisition"};

static const QLatin1String REL_START{"start"};
// static const QLatin1String REL_SUBSECTION{"subsection"};
// static const QLatin1String REL_COLLECTION{"collection"};
// static const QLatin1String REL_PREVIEW{"preview"};
// static const QLatin1String REL_REPLIES{"replies"};
// static const QLatin1String REL_RELATED{"related"};
// static const QLatin1String REL_PREVIOUS{"previous"};
// static const QLatin1String REL_NEXT{"next"};
// static const QLatin1String REL_FIRST{"first"};
// static const QLatin1String REL_LAST{"last"};
static const QLatin1String REL_UP{"up"};
static const QLatin1String REL_SELF{"self"};
static const QLatin1String REL_ALTERNATE{"alternate"};
static const QLatin1String ATTR_CURRENCY_CODE{"currencycode"};
// static const QLatin1String FEED_COMPLETE{"fh:complete"};
// static const QLatin1String THREAD_COUNT{"count"};

static const QLatin1String OPENSEARCH_NS{"http://a9.com/-/spec/opensearch/1.1/"};
static const QLatin1String OPENSEARCH_MT{"application/opensearchdescription+xml"};
static const QLatin1String REL_SEARCH{"search"};

static const QLatin1String OPENSEARCH_EL_URL{"Url"};
static const QLatin1String OPENSEARCH_ATTR_TYPE{"type"};
static const QLatin1String OPENSEARCH_ATTR_TEMPLATE{"template"};
static const QLatin1String OPENSEARCH_SEARCH_TERMS{"searchTerms"};
static const QLatin1String OPENSEARCH_COUNT{"count"};
static const QLatin1String OPENSEARCH_START_INDEX{"startIndex"};
static const QLatin1String OPENSEARCH_START_PAGE{"startPage"};

static const QLatin1String HTML_MT{"text/html"};

static const QLatin1String KEY_MIME_TYPE{"data##mimetype="};
static const QLatin1String KEY_URL{"data##url="};
static const QLatin1String KEY_LANGUAGE{"data##language="};

class OPDSProviderPrivate
{
public:
    OPDSProviderPrivate(OPDSProvider *qq)
        : q(qq)
        , initialized(false)
        , loadingExtraDetails(false)
    {
    }
    OPDSProvider *q;
    QString providerId;
    QString providerName;
    QUrl iconUrl;
    bool initialized;

    /***
     * OPDS catalogs consist of many small atom feeds. This variable
     * tracks which atom feed to load.
     */
    QUrl currentUrl;
    // partial url identifying the self. This is necessary to resolve relative links.
    QString selfUrl;

    QDateTime currentTime;
    bool loadingExtraDetails;

    XmlLoader *xmlLoader;

    Entry::List cachedEntries;
    Provider::SearchRequest currentRequest;

    QUrl openSearchDocumentURL;
    QString openSearchTemplate;

    // Generate an opensearch string.
    QUrl openSearchStringForRequest(const KNSCore::Provider::SearchRequest &request)
    {
        {
            QUrl searchUrl = QUrl(openSearchTemplate);

            QUrlQuery templateQuery(searchUrl);
            QUrlQuery query;

            for (QPair<QString, QString> key : templateQuery.queryItems()) {
                if (key.second.contains(OPENSEARCH_SEARCH_TERMS)) {
                    query.addQueryItem(key.first, request.searchTerm);
                } else if (key.second.contains(OPENSEARCH_COUNT)) {
                    query.addQueryItem(key.first, QString::number(request.pageSize));
                } else if (key.second.contains(OPENSEARCH_START_PAGE)) {
                    query.addQueryItem(key.first, QString::number(request.page));
                } else if (key.second.contains(OPENSEARCH_START_INDEX)) {
                    query.addQueryItem(key.first, QString::number(request.page * request.pageSize));
                }
            }
            searchUrl.setQuery(query);
            return searchUrl;
        }
    }

    // Handle URL resolving.
    QUrl fixRelativeUrl(QString urlPart)
    {
        QUrl query = QUrl(urlPart);
        if (query.isRelative()) {
            if (selfUrl.isEmpty() || QUrl(selfUrl).isRelative()) {
                return currentUrl.resolved(query);
            } else {
                return QUrl(selfUrl).resolved(query);
            }
        }
        return query;
    };

    Entry::List installedEntries() const {{Entry::List entries;
    for (const Entry &entry : std::as_const(cachedEntries)) {
        if (entry.status() == KNSCore::Entry::Installed || entry.status() == KNSCore::Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}
};

void slotLoadingFailed()
{
    qCWarning(KNEWSTUFFCORE) << "OPDS Loading failed for" << currentUrl;
    Q_EMIT q->loadingFailed(currentRequest);
};

// Parse the opensearch configuration document.
// https://github.com/dewitt/opensearch
void parseOpenSearchDocument(const QDomDocument &doc){{openSearchTemplate = QString();
if (doc.documentElement().attribute(QStringLiteral("xmlns")) != OPENSEARCH_NS) {
    qCWarning(KNEWSTUFFCORE) << "Opensearch link does not point at document with opensearch namespace" << openSearchDocumentURL;
    return;
}
QDomElement el = doc.documentElement().firstChildElement(OPENSEARCH_EL_URL);
while (!el.isNull()) {
    if (el.attribute(OPENSEARCH_ATTR_TYPE).contains(OPDS_ATOM_MT)) {
        if (openSearchTemplate.isEmpty() || el.attribute(OPENSEARCH_ATTR_TYPE).contains(OPDS_PROFILE)) {
            openSearchTemplate = el.attribute(OPENSEARCH_ATTR_TEMPLATE);
        }
    }

    el = el.nextSiblingElement(OPENSEARCH_EL_URL);
}
}
}
;

/**
 * @brief parseFeedData
 * The main parsing function of this provider. Receives a QDomDocument
 * and parses that with Syndication's atom reader.
 * @param doc
 */
void parseFeedData(const QDomDocument &doc)
{
    Syndication::DocumentSource source(doc.toByteArray(), currentUrl.toString());
    Syndication::Atom::Parser parser;
    Syndication::Atom::FeedDocumentPtr feedDoc = parser.parse(source).staticCast<Syndication::Atom::FeedDocument>();

    QString fullEntryMimeType = QStringList({OPDS_ATOM_MT, OPDS_TYPE_ENTRY, OPDS_PROFILE}).join(QStringLiteral(";"));

    if (!feedDoc->isValid()) {
        qCWarning(KNEWSTUFFCORE) << "OPDS Feed at" << currentUrl << "not valid";
        Q_EMIT q->loadingFailed(currentRequest);
        return;
    }
    if (!feedDoc->title().isEmpty()) {
        providerName = feedDoc->title();
    }
    if (!feedDoc->icon().isEmpty()) {
        iconUrl = QUrl(fixRelativeUrl(feedDoc->icon()));
    }

    Entry::List entries;
    QList<OPDSProvider::SearchPreset> presets;

    {
        OPDSProvider::SearchPreset preset;
        preset.providerId = providerId;
        OPDSProvider::SearchRequest request;
        request.searchTerm = providerId;
        preset.request = request;
        preset.type = Provider::SearchPresetTypes::Start;
        presets.append(preset);
    }

    // find the self link first!
    selfUrl.clear();
    for (auto link : feedDoc->links()) {
        if (link.rel().contains(REL_SELF)) {
            selfUrl = link.href();
        }
    }

    for (auto link : feedDoc->links()) {
        // There will be a number of links toplevel, amongst which probably a lot of sortorder and navigation links.
        if (link.rel() == REL_SEARCH && link.type() == OPENSEARCH_MT) {
            std::function<void(Syndication::Atom::Link)> osdUrlLoader;
            osdUrlLoader = [this, &osdUrlLoader](Syndication::Atom::Link theLink) {
                openSearchDocumentURL = fixRelativeUrl(theLink.href());
                xmlLoader = new XmlLoader(q);

                QObject::connect(xmlLoader, &XmlLoader::signalLoaded, q, [this](const QDomDocument &doc) {
                    q->d->parseOpenSearchDocument(doc);
                });
                QObject::connect(xmlLoader, &XmlLoader::signalFailed, q, [this]() {
                    qCWarning(KNEWSTUFFCORE) << "OpenSearch XML Document Loading failed" << openSearchDocumentURL;
                });
                QObject::connect(
                    xmlLoader,
                    &XmlLoader::signalHttpError,
                    q,
                    [this, &osdUrlLoader, theLink](int status, QList<QNetworkReply::RawHeaderPair> rawHeaders) { // clazy:exclude=lambda-in-connect
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
                            // clazy:exclude=lambda-in-connect
                            QTimer::singleShot(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch(), q, [&osdUrlLoader, theLink]() {
                                osdUrlLoader(theLink);
                            });
                            // if it's a matter of a human moment's worth of seconds, just reload
                            if (retryAfter.toSecsSinceEpoch() - QDateTime::currentSecsSinceEpoch() > 2) {
                                // more than that, spit out TryAgainLaterError to let the user know what we're doing with their time
                                static const KFormat formatter;
                                Q_EMIT q->signalErrorCode(
                                    KNSCore::ErrorCode::TryAgainLaterError,
                                    i18n("The service is currently undergoing maintenance and is expected to be back in %1.",
                                         formatter.formatSpelloutDuration(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch())),
                                    {retryAfter});
                            }
                        }
                    });

                xmlLoader->load(openSearchDocumentURL);
            };
        } else if (link.type().contains(OPDS_PROFILE) && link.rel() != REL_SELF) {
            OPDSProvider::SearchPreset preset;
            preset.providerId = providerId;
            preset.displayName = link.title();
            OPDSProvider::SearchRequest request;
            request.searchTerm = fixRelativeUrl(link.href()).toString();
            preset.request = request;
            if (link.rel() == REL_START) {
                preset.type = Provider::SearchPresetTypes::Root;
            } else if (link.rel() == OPDS_REL_FEATURED) {
                preset.type = Provider::SearchPresetTypes::Featured;
            } else if (link.rel() == OPDS_REL_SHELF) {
                preset.type = Provider::SearchPresetTypes::Shelf;
            } else if (link.rel() == OPDS_REL_SORT_NEW) {
                preset.type = Provider::SearchPresetTypes::New;
            } else if (link.rel() == OPDS_REL_SORT_POPULAR) {
                preset.type = Provider::SearchPresetTypes::Popular;
            } else if (link.rel() == REL_UP) {
                preset.type = Provider::SearchPresetTypes::FolderUp;
            } else if (link.rel() == OPDS_REL_CRAWL) {
                preset.type = Provider::SearchPresetTypes::AllEntries;
            } else if (link.rel() == OPDS_REL_RECOMMENDED) {
                preset.type = Provider::SearchPresetTypes::Recommended;
            } else if (link.rel() == OPDS_REL_SUBSCRIPTIONS) {
                preset.type = Provider::SearchPresetTypes::Subscription;
            } else {
                preset.type = Provider::SearchPresetTypes::NoPresetType;
                if (preset.displayName.isEmpty()) {
                    preset.displayName = link.rel();
                }
            }
            presets.append(preset);
        }
    }
    TagsFilterChecker downloadTagChecker(q->downloadTagFilter());
    TagsFilterChecker entryTagChecker(q->tagFilter());

    for (int i = 0; i < feedDoc->entries().size(); i++) {
        Syndication::Atom::Entry feedEntry = feedDoc->entries().at(i);

        Entry entry;
        entry.setName(feedEntry.title());
        entry.setProviderId(providerId);
        entry.setUniqueId(feedEntry.id());

        entry.setStatus(KNSCore::Entry::Invalid);
        for (const Entry &cachedEntry : std::as_const(cachedEntries)) {
            if (entry.uniqueId() == cachedEntry.uniqueId()) {
                entry = cachedEntry;
                break;
            }
        }

        // This is a bit of a pickle: atom feeds can have multiple categories.
        // but these categories are not specifically tags...
        QStringList entryTags;
        for (int j = 0; j < feedEntry.categories().size(); j++) {
            QString tag = feedEntry.categories().at(j).label();
            if (tag.isEmpty()) {
                tag = feedEntry.categories().at(j).term();
            }
            entryTags.append(tag);
        }
        if (entryTagChecker.filterAccepts(entryTags)) {
            entry.setTags(entryTags);
        } else {
            continue;
        }
        // Same issue with author...
        for (int j = 0; j < feedEntry.authors().size(); j++) {
            Author author;
            Syndication::Atom::Person person = feedEntry.authors().at(j);
            author.setId(person.uri());
            author.setName(person.name());
            author.setEmail(person.email());
            author.setHomepage(person.uri());
            entry.setAuthor(author);
        }
        entry.setLicense(feedEntry.rights());
        if (feedEntry.content().isEscapedHTML()) {
            entry.setSummary(feedEntry.content().childNodesAsXML());
        } else {
            entry.setSummary(feedEntry.content().asString());
        }
        entry.setShortSummary(feedEntry.summary());

        int counterThumbnails = 0;
        int counterImages = 0;
        QString groupEntryUrl;
        for (int j = 0; j < feedEntry.links().size(); j++) {
            Syndication::Atom::Link link = feedEntry.links().at(j);

            KNSCore::Entry::DownloadLinkInformation download;
            download.id = entry.downloadLinkCount() + 1;
            // Linkrelations can have multiple values, expressed as something like... rel="me nofollow alternate".
            QStringList linkRelation = link.rel().split(QStringLiteral(" "));

            QStringList tags;
            tags.append(KEY_MIME_TYPE + link.type());
            if (!link.hrefLanguage().isEmpty()) {
                tags.append(KEY_LANGUAGE + link.hrefLanguage());
            }
            QString linkUrl = fixRelativeUrl(link.href()).toString();
            tags.append(KEY_URL + linkUrl);
            download.name = link.title();
            download.size = link.length() / 1000;
            download.tags = tags;
            download.isDownloadtypeLink = false;

            if (link.rel().startsWith(OPDS_REL_ACQUISITION)) {
                if (link.title().isEmpty()) {
                    QStringList l;
                    l.append(link.type());
                    l.append(QStringLiteral("(") + link.rel().split(QStringLiteral("/")).last() + QStringLiteral(")"));
                    download.name = l.join(QStringLiteral(" "));
                }

                if (!downloadTagChecker.filterAccepts(download.tags)) {
                    continue;
                }

                if (linkRelation.contains(OPDS_REL_AC_BORROW) || linkRelation.contains(OPDS_REL_AC_SUBSCRIBE) || linkRelation.contains(OPDS_REL_AC_BUY)) {
                    // TODO we don't support borrow, buy and subscribe right now, requires authentication.
                    continue;

                } else if (linkRelation.contains(OPDS_REL_ACQUISITION) || linkRelation.contains(OPDS_REL_AC_OPEN_ACCESS)) {
                    download.isDownloadtypeLink = true;

                    if (entry.status() != KNSCore::Entry::Installed && entry.status() != KNSCore::Entry::Updateable) {
                        entry.setStatus(KNSCore::Entry::Downloadable);
                    }

                    entry.setEntryType(Entry::CatalogEntry);
                }
                // TODO, support preview relation, but this requires we show that an entry is otherwise paid for in the UI.

                for (QDomElement el : feedEntry.elementsByTagName(OPDS_EL_PRICE)) {
                    QLocale locale;
                    download.priceAmount = locale.toCurrencyString(el.text().toFloat(), el.attribute(ATTR_CURRENCY_CODE));
                }
                // There's an 'opds:indirectaquistition' element that gives extra metadata about bundles.
                entry.appendDownloadLinkInformation(download);

            } else if (link.rel().startsWith(OPDS_REL_IMAGE)) {
                if (link.rel() == OPDS_REL_THUMBNAIL) {
                    entry.setPreviewUrl(linkUrl, KNSCore::Entry::PreviewType(counterThumbnails));
                    counterThumbnails += 1;
                } else {
                    entry.setPreviewUrl(linkUrl, KNSCore::Entry::PreviewType(counterImages + 3));
                    counterImages += 1;
                }

            } else {
                // This could be anything from a more info link, to navigation links, to links to the outside world.
                // Todo: think of using link rel's 'replies', 'payment'(donation) and 'version-history'.

                if (link.type().startsWith(OPDS_ATOM_MT)) {
                    if (link.type() == fullEntryMimeType) {
                        entry.appendDownloadLinkInformation(download);
                    } else {
                        groupEntryUrl = linkUrl;
                    }

                } else if (link.type() == HTML_MT && linkRelation.contains(REL_ALTERNATE)) {
                    entry.setHomepage(QUrl(linkUrl));

                } else if (downloadTagChecker.filterAccepts(download.tags)) {
                    entry.appendDownloadLinkInformation(download);
                }
            }
        }

        // Todo:
        // feedEntry.elementsByTagName( dc:terms:issued ) is the official initial release date.
        // published is the released date of the opds catalog item, updated for the opds catalog item update.
        // maybe we should make sure to also check dc:terms:modified?
        // QDateTime date = QDateTime::fromSecsSinceEpoch(feedEntry.published());

        QDateTime date = QDateTime::fromSecsSinceEpoch(feedEntry.updated());

        if (entry.releaseDate().isNull()) {
            entry.setReleaseDate(date.date());
        }

        if (entry.status() != KNSCore::Entry::Invalid) {
            entry.setPayload(QString());
            // Gutenberg doesn't do versioning in the opds, so it's update value is unreliable,
            // even though openlib and standard do use it properly. We'll instead doublecheck that
            // the new time is larger than 6min since we requested the feed.
            if (date.secsTo(currentTime) > 360) {
                if (entry.releaseDate() < date.date()) {
                    entry.setUpdateReleaseDate(date.date());
                    if (entry.status() == KNSCore::Entry::Installed) {
                        entry.setStatus(KNSCore::Entry::Updateable);
                    }
                }
            }
        }
        if (counterThumbnails == 0) {
            // fallback.
            if (!feedDoc->icon().isEmpty()) {
                entry.setPreviewUrl(fixRelativeUrl(feedDoc->icon()).toString());
            }
        }

        if (entry.downloadLinkCount() == 0) {
            if (groupEntryUrl.isEmpty()) {
                continue;
            } else {
                entry.setEntryType(Entry::GroupEntry);
                entry.setPayload(groupEntryUrl);
            }
        }

        entries.append(entry);
    }

    if (loadingExtraDetails) {
        Q_EMIT q->entryDetailsLoaded(entries.first());
        loadingExtraDetails = false;
    } else {
        Q_EMIT q->loadingFinished(currentRequest, entries);
    }
    Q_EMIT q->searchPresetsLoaded(presets);
};
}
;

OPDSProvider::OPDSProvider()
    : d(new OPDSProviderPrivate(this))
{
}

OPDSProvider::~OPDSProvider() = default;

QString OPDSProvider::id() const
{
    return d->providerId;
}

QString OPDSProvider::name() const
{
    return d->providerName;
}

QUrl OPDSProvider::icon() const
{
    return d->iconUrl;
}

void OPDSProvider::loadEntries(const KNSCore::Provider::SearchRequest &request)
{
    d->currentRequest = request;

    if (request.filter == Installed) {
        Q_EMIT loadingFinished(request, d->installedEntries());
        return;
    } else if (request.filter == Provider::ExactEntryId) {
        for (Entry entry : d->cachedEntries) {
            if (entry.uniqueId() == request.searchTerm) {
                loadEntryDetails(entry);
            }
        }
    } else {
        if (QUrl(request.searchTerm).scheme().startsWith(QStringLiteral("http"))) {
            d->currentUrl = QUrl(request.searchTerm);
        } else if (!d->openSearchTemplate.isEmpty() && !request.searchTerm.isEmpty()) {
            // We should check if there's an opensearch implementation, and see if we can funnel search
            // requests to that.
            d->currentUrl = d->openSearchStringForRequest(request);
        }

        // TODO request: check if entries is above pagesize*index, otherwise load next page.

        QUrl url = d->currentUrl;
        if (!url.isEmpty()) {
            qCDebug(KNEWSTUFFCORE) << "requesting url" << url;
            d->xmlLoader = new XmlLoader(this);
            d->currentTime = QDateTime::currentDateTime();
            d->loadingExtraDetails = false;
            connect(d->xmlLoader, &XmlLoader::signalLoaded, this, [this](const QDomDocument &doc) {
                d->parseFeedData(doc);
            });
            connect(d->xmlLoader, &XmlLoader::signalFailed, this, [this]() {
                d->slotLoadingFailed();
            });
            d->xmlLoader->load(url);
        } else {
            Q_EMIT loadingFailed(request);
        }
    }
}

void OPDSProvider::loadEntryDetails(const Entry &entry)
{
    QUrl url;
    QString entryMimeType = QStringList({OPDS_ATOM_MT, OPDS_TYPE_ENTRY, OPDS_PROFILE}).join(QStringLiteral(";"));
    for (auto link : entry.downloadLinkInformationList()) {
        if (link.tags.contains(KEY_MIME_TYPE + entryMimeType)) {
            for (QString string : link.tags) {
                if (string.startsWith(KEY_URL)) {
                    url = QUrl(string.split(QStringLiteral("=")).last());
                }
            }
        }
    }
    if (!url.isEmpty()) {
        d->xmlLoader = new XmlLoader(this);
        d->currentTime = QDateTime::currentDateTime();
        d->loadingExtraDetails = true;
        connect(d->xmlLoader, &XmlLoader::signalLoaded, this, [this](const QDomDocument &doc) {
            d->parseFeedData(doc);
        });
        connect(d->xmlLoader, &XmlLoader::signalFailed, this, [this]() {
            d->slotLoadingFailed();
        });
        d->xmlLoader->load(url);
    }
}

void OPDSProvider::loadPayloadLink(const KNSCore::Entry &entry, int linkNumber)
{
    KNSCore::Entry copy = entry;
    for (auto downloadInfo : entry.downloadLinkInformationList()) {
        if (downloadInfo.id == linkNumber) {
            for (QString string : downloadInfo.tags) {
                if (string.startsWith(KEY_URL)) {
                    copy.setPayload(string.split(QStringLiteral("=")).last());
                }
            }
        }
    }
    Q_EMIT payloadLinkLoaded(copy);
}

bool OPDSProvider::setProviderXML(const QDomElement &xmldata)
{
    if (xmldata.tagName() != QLatin1String("provider")) {
        return false;
    }
    d->providerId = xmldata.attribute(QStringLiteral("downloadurl"));

    QUrl iconurl(xmldata.attribute(QStringLiteral("icon")));
    if (!iconurl.isValid()) {
        iconurl = QUrl::fromLocalFile(xmldata.attribute(QStringLiteral("icon")));
    }
    d->iconUrl = iconurl;

    QDomNode n;
    for (n = xmldata.firstChild(); !n.isNull(); n = n.nextSibling()) {
        QDomElement e = n.toElement();
        if (e.tagName() == QLatin1String("title")) {
            d->providerName = e.text().trimmed();
        }
    }

    d->currentUrl = QUrl(d->providerId);
    QTimer::singleShot(0, this, [this]() {
        d->initialized = true;
        Q_EMIT providerInitialized(this);
    });
    return true;
}

bool OPDSProvider::isInitialized() const
{
    return d->initialized;
}

void OPDSProvider::setCachedEntries(const KNSCore::Entry::List &cachedEntries)
{
    d->cachedEntries = cachedEntries;
}
}

#include "moc_opdsprovider_p.cpp"
