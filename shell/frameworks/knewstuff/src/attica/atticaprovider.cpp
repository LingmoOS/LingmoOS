/*
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "atticaprovider_p.h"

#include "commentsmodel.h"
#include "entry_p.h"
#include "question.h"
#include "tagsfilterchecker.h"

#include <KFormat>
#include <KLocalizedString>
#include <QCollator>
#include <QDomDocument>
#include <knewstuffcore_debug.h>

#include <attica/accountbalance.h>
#include <attica/config.h>
#include <attica/content.h>
#include <attica/downloaditem.h>
#include <attica/listjob.h>
#include <attica/person.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

using namespace Attica;

namespace KNSCore
{
AtticaProvider::AtticaProvider(const QStringList &categories, const QString &additionalAgentInformation)
    : mEntryJob(nullptr)
    , mInitialized(false)
{
    // init categories map with invalid categories
    for (const QString &category : categories) {
        mCategoryMap.insert(category, Attica::Category());
    }

    connect(&m_providerManager, &ProviderManager::providerAdded, this, [this, additionalAgentInformation](const Attica::Provider &provider) {
        providerLoaded(provider);
        m_provider.setAdditionalAgentInformation(additionalAgentInformation);
    });
    connect(&m_providerManager, &ProviderManager::authenticationCredentialsMissing, this, &AtticaProvider::onAuthenticationCredentialsMissing);
}

AtticaProvider::AtticaProvider(const Attica::Provider &provider, const QStringList &categories, const QString &additionalAgentInformation)
    : mEntryJob(nullptr)
    , mInitialized(false)
{
    // init categories map with invalid categories
    for (const QString &category : categories) {
        mCategoryMap.insert(category, Attica::Category());
    }
    providerLoaded(provider);
    m_provider.setAdditionalAgentInformation(additionalAgentInformation);
}

QString AtticaProvider::id() const
{
    return m_providerId;
}

void AtticaProvider::onAuthenticationCredentialsMissing(const Attica::Provider &)
{
    qCDebug(KNEWSTUFFCORE) << "Authentication missing!";
    // FIXME Show authentication dialog
}

bool AtticaProvider::setProviderXML(const QDomElement &xmldata)
{
    if (xmldata.tagName() != QLatin1String("provider")) {
        return false;
    }

    // FIXME this is quite ugly, repackaging the xml into a string
    QDomDocument doc(QStringLiteral("temp"));
    qCDebug(KNEWSTUFFCORE) << "setting provider xml" << doc.toString();

    doc.appendChild(xmldata.cloneNode(true));
    m_providerManager.addProviderFromXml(doc.toString());

    if (!m_providerManager.providers().isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "base url of attica provider:" << m_providerManager.providers().constLast().baseUrl().toString();
    } else {
        qCCritical(KNEWSTUFFCORE) << "Could not load provider.";
        return false;
    }
    return true;
}

void AtticaProvider::setCachedEntries(const KNSCore::Entry::List &cachedEntries)
{
    mCachedEntries = cachedEntries;
}

void AtticaProvider::providerLoaded(const Attica::Provider &provider)
{
    setName(provider.name());
    setIcon(provider.icon());
    qCDebug(KNEWSTUFFCORE) << "Added provider: " << provider.name();

    m_provider = provider;
    m_provider.setAdditionalAgentInformation(name());
    m_providerId = provider.baseUrl().host();

    Attica::ListJob<Attica::Category> *job = m_provider.requestCategories();
    connect(job, &BaseJob::finished, this, &AtticaProvider::listOfCategoriesLoaded);
    job->start();
}

void AtticaProvider::listOfCategoriesLoaded(Attica::BaseJob *listJob)
{
    if (!jobSuccess(listJob)) {
        return;
    }

    qCDebug(KNEWSTUFFCORE) << "loading categories: " << mCategoryMap.keys();

    auto *job = static_cast<Attica::ListJob<Attica::Category> *>(listJob);
    const Category::List categoryList = job->itemList();

    QList<CategoryMetadata> categoryMetadataList;
    for (const Category &category : categoryList) {
        if (mCategoryMap.contains(category.name())) {
            qCDebug(KNEWSTUFFCORE) << "Adding category: " << category.name() << category.displayName();
            // If there is only the placeholder category, replace it
            if (mCategoryMap.contains(category.name()) && !mCategoryMap.value(category.name()).isValid()) {
                mCategoryMap.replace(category.name(), category);
            } else {
                mCategoryMap.insert(category.name(), category);
            }

            CategoryMetadata categoryMetadata;
            categoryMetadata.id = category.id();
            categoryMetadata.name = category.name();
            categoryMetadata.displayName = category.displayName();
            categoryMetadataList << categoryMetadata;
        }
    }
    std::sort(categoryMetadataList.begin(),
              categoryMetadataList.end(),
              [](const AtticaProvider::CategoryMetadata &i, const AtticaProvider::CategoryMetadata &j) -> bool {
                  const QString a(i.displayName.isEmpty() ? i.name : i.displayName);
                  const QString b(j.displayName.isEmpty() ? j.name : j.displayName);

                  return (QCollator().compare(a, b) < 0);
              });

    bool correct = false;
    for (auto it = mCategoryMap.cbegin(), itEnd = mCategoryMap.cend(); it != itEnd; ++it) {
        if (!it.value().isValid()) {
            qCWarning(KNEWSTUFFCORE) << "Could not find category" << it.key();
        } else {
            correct = true;
        }
    }

    if (correct) {
        mInitialized = true;
        Q_EMIT providerInitialized(this);
        Q_EMIT categoriesMetadataLoded(categoryMetadataList);
    } else {
        Q_EMIT signalErrorCode(KNSCore::ErrorCode::ConfigFileError, i18n("All categories are missing"), QVariant());
    }
}

bool AtticaProvider::isInitialized() const
{
    return mInitialized;
}

void AtticaProvider::loadEntries(const KNSCore::Provider::SearchRequest &request)
{
    if (mEntryJob) {
        mEntryJob->abort();
        mEntryJob = nullptr;
    }

    mCurrentRequest = request;
    switch (request.filter) {
    case None:
        break;
    case ExactEntryId: {
        ItemJob<Content> *job = m_provider.requestContent(request.searchTerm);
        job->setProperty("providedEntryId", request.searchTerm);
        connect(job, &BaseJob::finished, this, &AtticaProvider::detailsLoaded);
        job->start();
        return;
    }
    case Installed:
        if (request.page == 0) {
            Q_EMIT loadingFinished(request, installedEntries());
        } else {
            Q_EMIT loadingFinished(request, Entry::List());
        }
        return;
    case Updates:
        checkForUpdates();
        return;
    }

    Attica::Provider::SortMode sorting = atticaSortMode(request.sortMode);
    Attica::Category::List categoriesToSearch;

    if (request.categories.isEmpty()) {
        // search in all categories
        categoriesToSearch = mCategoryMap.values();
    } else {
        categoriesToSearch.reserve(request.categories.size());
        for (const QString &categoryName : std::as_const(request.categories)) {
            categoriesToSearch.append(mCategoryMap.values(categoryName));
        }
    }

    ListJob<Content> *job = m_provider.searchContents(categoriesToSearch, request.searchTerm, sorting, request.page, request.pageSize);
    job->setProperty("searchRequest", QVariant::fromValue(request));
    connect(job, &BaseJob::finished, this, &AtticaProvider::categoryContentsLoaded);

    mEntryJob = job;
    job->start();
}

void AtticaProvider::checkForUpdates()
{
    if (mCachedEntries.isEmpty()) {
        Q_EMIT loadingFinished(mCurrentRequest, {});
    }

    for (const Entry &e : std::as_const(mCachedEntries)) {
        ItemJob<Content> *job = m_provider.requestContent(e.uniqueId());
        connect(job, &BaseJob::finished, this, &AtticaProvider::detailsLoaded);
        m_updateJobs.insert(job);
        job->start();
        qCDebug(KNEWSTUFFCORE) << "Checking for update: " << e.name();
    }
}

void AtticaProvider::loadEntryDetails(const KNSCore::Entry &entry)
{
    ItemJob<Content> *job = m_provider.requestContent(entry.uniqueId());
    connect(job, &BaseJob::finished, this, &AtticaProvider::detailsLoaded);
    job->start();
}

void AtticaProvider::detailsLoaded(BaseJob *job)
{
    if (jobSuccess(job)) {
        auto *contentJob = static_cast<ItemJob<Content> *>(job);
        Content content = contentJob->result();
        Entry entry = entryFromAtticaContent(content);
        entry.setEntryRequestedId(job->property("providedEntryId").toString()); // The ResultsStream should still known that this entry was for its query
        Q_EMIT entryDetailsLoaded(entry);
        qCDebug(KNEWSTUFFCORE) << "check update finished: " << entry.name();
    }

    if (m_updateJobs.remove(job) && m_updateJobs.isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "check update finished.";
        QList<Entry> updatable;
        for (const Entry &entry : std::as_const(mCachedEntries)) {
            if (entry.status() == KNSCore::Entry::Updateable) {
                updatable.append(entry);
            }
        }
        Q_EMIT loadingFinished(mCurrentRequest, updatable);
    }
}

void AtticaProvider::categoryContentsLoaded(BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }

    auto *listJob = static_cast<ListJob<Content> *>(job);
    const Content::List contents = listJob->itemList();

    Entry::List entries;
    TagsFilterChecker checker(tagFilter());
    TagsFilterChecker downloadschecker(downloadTagFilter());
    for (const Content &content : contents) {
        if (!content.isValid()) {
            qCDebug(KNEWSTUFFCORE)
                << "Filtered out an invalid entry. This suggests something is not right on the originating server. Please contact the administrators of"
                << name() << "and inform them there is an issue with content in the category or categories" << mCurrentRequest.categories;
            continue;
        }
        if (checker.filterAccepts(content.tags())) {
            bool filterAcceptsDownloads = true;
            if (content.downloads() > 0) {
                filterAcceptsDownloads = false;
                const QList<Attica::DownloadDescription> descs = content.downloadUrlDescriptions();
                for (const Attica::DownloadDescription &dli : descs) {
                    if (downloadschecker.filterAccepts(dli.tags())) {
                        filterAcceptsDownloads = true;
                        break;
                    }
                }
            }
            if (filterAcceptsDownloads) {
                mCachedContent.insert(content.id(), content);
                entries.append(entryFromAtticaContent(content));
            } else {
                qCDebug(KNEWSTUFFCORE) << "Filter has excluded" << content.name() << "on download filter" << downloadTagFilter();
            }
        } else {
            qCDebug(KNEWSTUFFCORE) << "Filter has excluded" << content.name() << "on entry filter" << tagFilter();
        }
    }

    qCDebug(KNEWSTUFFCORE) << "loaded: " << mCurrentRequest.hashForRequest() << " count: " << entries.size();
    Q_EMIT loadingFinished(mCurrentRequest, entries);
    mEntryJob = nullptr;
}

Attica::Provider::SortMode AtticaProvider::atticaSortMode(SortMode sortMode)
{
    switch (sortMode) {
    case Newest:
        return Attica::Provider::Newest;
    case Alphabetical:
        return Attica::Provider::Alphabetical;
    case Downloads:
        return Attica::Provider::Downloads;
    default:
        return Attica::Provider::Rating;
    }
}

void AtticaProvider::loadPayloadLink(const KNSCore::Entry &entry, int linkId)
{
    Attica::Content content = mCachedContent.value(entry.uniqueId());
    const DownloadDescription desc = content.downloadUrlDescription(linkId);

    if (desc.hasPrice()) {
        // Ask for balance, then show information...
        ItemJob<AccountBalance> *job = m_provider.requestAccountBalance();
        connect(job, &BaseJob::finished, this, &AtticaProvider::accountBalanceLoaded);
        mDownloadLinkJobs[job] = qMakePair(entry, linkId);
        job->start();

        qCDebug(KNEWSTUFFCORE) << "get account balance";
    } else {
        ItemJob<DownloadItem> *job = m_provider.downloadLink(entry.uniqueId(), QString::number(linkId));
        connect(job, &BaseJob::finished, this, &AtticaProvider::downloadItemLoaded);
        mDownloadLinkJobs[job] = qMakePair(entry, linkId);
        job->start();

        qCDebug(KNEWSTUFFCORE) << " link for " << entry.uniqueId();
    }
}

void AtticaProvider::loadComments(const Entry &entry, int commentsPerPage, int page)
{
    ListJob<Attica::Comment> *job = m_provider.requestComments(Attica::Comment::ContentComment, entry.uniqueId(), QStringLiteral("0"), page, commentsPerPage);
    connect(job, &BaseJob::finished, this, &AtticaProvider::loadedComments);
    job->start();
}

QList<std::shared_ptr<KNSCore::Comment>> getCommentsList(const Attica::Comment::List &comments, std::shared_ptr<KNSCore::Comment> parent)
{
    QList<std::shared_ptr<KNSCore::Comment>> knsComments;
    for (const Attica::Comment &comment : comments) {
        qCDebug(KNEWSTUFFCORE) << "Appending comment with id" << comment.id() << ", which has" << comment.childCount() << "children";
        auto knsComment = std::make_shared<KNSCore::Comment>();
        knsComment->id = comment.id();
        knsComment->subject = comment.subject();
        knsComment->text = comment.text();
        knsComment->childCount = comment.childCount();
        knsComment->username = comment.user();
        knsComment->date = comment.date();
        knsComment->score = comment.score();
        knsComment->parent = parent;
        knsComments << knsComment;
        if (comment.childCount() > 0) {
            qCDebug(KNEWSTUFFCORE) << "Getting more comments, as this one has children, and we currently have this number of comments:" << knsComments.count();
            knsComments << getCommentsList(comment.children(), knsComment);
            qCDebug(KNEWSTUFFCORE) << "After getting the children, we now have the following number of comments:" << knsComments.count();
        }
    }
    return knsComments;
}

void AtticaProvider::loadedComments(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ListJob<Attica::Comment> *>(baseJob);
    Attica::Comment::List comments = job->itemList();

    QList<std::shared_ptr<KNSCore::Comment>> receivedComments = getCommentsList(comments, nullptr);
    Q_EMIT commentsLoaded(receivedComments);
}

void AtticaProvider::loadPerson(const QString &username)
{
    if (m_provider.hasPersonService()) {
        ItemJob<Attica::Person> *job = m_provider.requestPerson(username);
        job->setProperty("username", username);
        connect(job, &BaseJob::finished, this, &AtticaProvider::loadedPerson);
        job->start();
    }
}

void AtticaProvider::loadedPerson(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ItemJob<Attica::Person> *>(baseJob);
    Attica::Person person = job->result();

    auto author = std::make_shared<KNSCore::Author>();
    // This is a touch hack-like, but it ensures we actually have the data in case it is not returned by the server
    author->setId(job->property("username").toString());
    author->setName(QStringLiteral("%1 %2").arg(person.firstName(), person.lastName()).trimmed());
    author->setHomepage(person.homepage());
    author->setProfilepage(person.extendedAttribute(QStringLiteral("profilepage")));
    author->setAvatarUrl(person.avatarUrl());
    author->setDescription(person.extendedAttribute(QStringLiteral("description")));
    Q_EMIT personLoaded(author);
}

void AtticaProvider::loadBasics()
{
    Attica::ItemJob<Attica::Config> *configJob = m_provider.requestConfig();
    connect(configJob, &BaseJob::finished, this, &AtticaProvider::loadedConfig);
    configJob->start();
}

void AtticaProvider::loadedConfig(Attica::BaseJob *baseJob)
{
    if (jobSuccess(baseJob)) {
        auto *job = static_cast<ItemJob<Attica::Config> *>(baseJob);
        Attica::Config config = job->result();
        setVersion(config.version());
        setSupportsSsl(config.ssl());
        setContactEmail(config.contact());
        QString protocol{QStringLiteral("http")};
        if (config.ssl()) {
            protocol = QStringLiteral("https");
        }
        // There is usually no protocol in the website and host, but in case
        // there is, trust what's there
        if (config.website().contains(QLatin1String("://"))) {
            setWebsite(QUrl(config.website()));
        } else {
            setWebsite(QUrl(QLatin1String("%1://%2").arg(protocol).arg(config.website())));
        }
        if (config.host().contains(QLatin1String("://"))) {
            setHost(QUrl(config.host()));
        } else {
            setHost(QUrl(QLatin1String("%1://%2").arg(protocol).arg(config.host())));
        }
    }
}

void AtticaProvider::accountBalanceLoaded(Attica::BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ItemJob<AccountBalance> *>(baseJob);
    AccountBalance item = job->result();

    QPair<Entry, int> pair = mDownloadLinkJobs.take(job);
    Entry entry(pair.first);
    Content content = mCachedContent.value(entry.uniqueId());
    if (content.downloadUrlDescription(pair.second).priceAmount() < item.balance()) {
        qCDebug(KNEWSTUFFCORE) << "Your balance is greater than the price." << content.downloadUrlDescription(pair.second).priceAmount()
                               << " balance: " << item.balance();
        Question question;
        question.setEntry(entry);
        question.setQuestion(i18nc("the price of a download item, parameter 1 is the currency, 2 is the price",
                                   "This item costs %1 %2.\nDo you want to buy it?",
                                   item.currency(),
                                   content.downloadUrlDescription(pair.second).priceAmount()));
        if (question.ask() == Question::YesResponse) {
            ItemJob<DownloadItem> *job = m_provider.downloadLink(entry.uniqueId(), QString::number(pair.second));
            connect(job, &BaseJob::finished, this, &AtticaProvider::downloadItemLoaded);
            mDownloadLinkJobs[job] = qMakePair(entry, pair.second);
            job->start();
        } else {
            return;
        }
    } else {
        qCDebug(KNEWSTUFFCORE) << "You don't have enough money on your account!" << content.downloadUrlDescription(0).priceAmount()
                               << " balance: " << item.balance();
        Q_EMIT signalInformation(i18n("Your account balance is too low:\nYour balance: %1\nPrice: %2", //
                                      item.balance(),
                                      content.downloadUrlDescription(0).priceAmount()));
    }
}

void AtticaProvider::downloadItemLoaded(BaseJob *baseJob)
{
    if (!jobSuccess(baseJob)) {
        return;
    }

    auto *job = static_cast<ItemJob<DownloadItem> *>(baseJob);
    DownloadItem item = job->result();

    Entry entry = mDownloadLinkJobs.take(job).first;
    entry.setPayload(QString(item.url().toString()));
    Q_EMIT payloadLinkLoaded(entry);
}

Entry::List AtticaProvider::installedEntries() const
{
    Entry::List entries;
    for (const Entry &entry : std::as_const(mCachedEntries)) {
        if (entry.status() == KNSCore::Entry::Installed || entry.status() == KNSCore::Entry::Updateable) {
            entries.append(entry);
        }
    }
    return entries;
}

void AtticaProvider::vote(const Entry &entry, uint rating)
{
    PostJob *job = m_provider.voteForContent(entry.uniqueId(), rating);
    connect(job, &BaseJob::finished, this, &AtticaProvider::votingFinished);
    job->start();
}

void AtticaProvider::votingFinished(Attica::BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }
    Q_EMIT signalInformation(i18nc("voting for an item (good/bad)", "Your vote was recorded."));
}

void AtticaProvider::becomeFan(const Entry &entry)
{
    PostJob *job = m_provider.becomeFan(entry.uniqueId());
    connect(job, &BaseJob::finished, this, &AtticaProvider::becomeFanFinished);
    job->start();
}

void AtticaProvider::becomeFanFinished(Attica::BaseJob *job)
{
    if (!jobSuccess(job)) {
        return;
    }
    Q_EMIT signalInformation(i18n("You are now a fan."));
}

bool AtticaProvider::jobSuccess(Attica::BaseJob *job)
{
    if (job->metadata().error() == Attica::Metadata::NoError) {
        return true;
    }
    qCDebug(KNEWSTUFFCORE) << "job error: " << job->metadata().error() << " status code: " << job->metadata().statusCode() << job->metadata().message();

    if (job->metadata().error() == Attica::Metadata::NetworkError) {
        if (job->metadata().statusCode() == 503) {
            QDateTime retryAfter;
            static const QByteArray retryAfterKey{"Retry-After"};
            for (const QNetworkReply::RawHeaderPair &headerPair : job->metadata().headers()) {
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
            static const KFormat formatter;
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::TryAgainLaterError,
                                   i18n("The service is currently undergoing maintenance and is expected to be back in %1.",
                                        formatter.formatSpelloutDuration(retryAfter.toMSecsSinceEpoch() - QDateTime::currentMSecsSinceEpoch())),
                                   {retryAfter});
        } else {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::NetworkError,
                                   i18n("Network error %1: %2", job->metadata().statusCode(), job->metadata().statusString()),
                                   job->metadata().statusCode());
        }
    }
    if (job->metadata().error() == Attica::Metadata::OcsError) {
        if (job->metadata().statusCode() == 200) {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::OcsError,
                                   i18n("Too many requests to server. Please try again in a few minutes."),
                                   job->metadata().statusCode());
        } else if (job->metadata().statusCode() == 405) {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::OcsError,
                                   i18n("The Open Collaboration Services instance %1 does not support the attempted function.", name()),
                                   job->metadata().statusCode());
        } else {
            Q_EMIT signalErrorCode(KNSCore::ErrorCode::OcsError,
                                   i18n("Unknown Open Collaboration Service API error. (%1)", job->metadata().statusCode()),
                                   job->metadata().statusCode());
        }
    }

    if (auto searchRequestVar = job->property("searchRequest"); searchRequestVar.isValid()) {
        SearchRequest req = searchRequestVar.value<SearchRequest>();
        Q_EMIT loadingFailed(req);
    }
    return false;
}

Entry AtticaProvider::entryFromAtticaContent(const Attica::Content &content)
{
    Entry entry;

    entry.setProviderId(id());
    entry.setUniqueId(content.id());
    entry.setStatus(KNSCore::Entry::Downloadable);
    entry.setVersion(content.version());
    entry.setReleaseDate(content.updated().date());
    entry.setCategory(content.attribute(QStringLiteral("typeid")));

    int index = mCachedEntries.indexOf(entry);
    if (index >= 0) {
        Entry &cacheEntry = mCachedEntries[index];
        // check if updateable
        if (((cacheEntry.status() == KNSCore::Entry::Installed) || (cacheEntry.status() == KNSCore::Entry::Updateable))
            && ((cacheEntry.version() != entry.version()) || (cacheEntry.releaseDate() != entry.releaseDate()))) {
            cacheEntry.setStatus(KNSCore::Entry::Updateable);
            cacheEntry.setUpdateVersion(entry.version());
            cacheEntry.setUpdateReleaseDate(entry.releaseDate());
        }
        entry = cacheEntry;
    } else {
        mCachedEntries.append(entry);
    }

    entry.setName(content.name());
    entry.setHomepage(content.detailpage());
    entry.setRating(content.rating());
    entry.setNumberOfComments(content.numberOfComments());
    entry.setDownloadCount(content.downloads());
    entry.setNumberFans(content.attribute(QStringLiteral("fans")).toInt());
    entry.setDonationLink(content.attribute(QStringLiteral("donationpage")));
    entry.setKnowledgebaseLink(content.attribute(QStringLiteral("knowledgebasepage")));
    entry.setNumberKnowledgebaseEntries(content.attribute(QStringLiteral("knowledgebaseentries")).toInt());
    entry.setHomepage(content.detailpage());

    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("1")), Entry::PreviewSmall1);
    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("2")), Entry::PreviewSmall2);
    entry.setPreviewUrl(content.smallPreviewPicture(QStringLiteral("3")), Entry::PreviewSmall3);

    entry.setPreviewUrl(content.previewPicture(QStringLiteral("1")), Entry::PreviewBig1);
    entry.setPreviewUrl(content.previewPicture(QStringLiteral("2")), Entry::PreviewBig2);
    entry.setPreviewUrl(content.previewPicture(QStringLiteral("3")), Entry::PreviewBig3);

    entry.setLicense(content.license());
    Author author;
    author.setId(content.author());
    author.setName(content.author());
    author.setHomepage(content.attribute(QStringLiteral("profilepage")));
    entry.setAuthor(author);

    entry.setSource(Entry::Online);
    entry.setSummary(content.description());
    entry.setShortSummary(content.summary());
    entry.setChangelog(content.changelog());
    entry.setTags(content.tags());

    const QList<Attica::DownloadDescription> descs = content.downloadUrlDescriptions();
    entry.d->mDownloadLinkInformationList.clear();
    entry.d->mDownloadLinkInformationList.reserve(descs.size());
    for (const Attica::DownloadDescription &desc : descs) {
        entry.d->mDownloadLinkInformationList.append({.name = desc.name(),
                                                      .priceAmount = desc.priceAmount(),
                                                      .distributionType = desc.distributionType(),
                                                      .descriptionLink = desc.link(),
                                                      .id = desc.id(),
                                                      .isDownloadtypeLink = desc.type() == Attica::DownloadDescription::LinkDownload,
                                                      .size = desc.size(),
                                                      .tags = desc.tags(),
                                                      .version = desc.version()});
    }

    return entry;
}

} // namespace

#include "moc_atticaprovider_p.cpp"
