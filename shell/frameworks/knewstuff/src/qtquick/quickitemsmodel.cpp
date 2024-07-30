/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "quickitemsmodel.h"
#include "core/commentsmodel.h"
#include "downloadlinkinfo.h"
#include "itemsmodel.h"
#include "quickengine.h"

#include <KShell>
#include <QProcess>

class ItemsModelPrivate
{
public:
    ItemsModelPrivate(ItemsModel *qq)
        : q(qq)
        , model(nullptr)
        , engine(nullptr)
    {
    }
    ItemsModel *q;
    KNSCore::ItemsModel *model;
    Engine *engine;

    QHash<QString, KNSCore::CommentsModel *> commentsModels;

    bool initModel()
    {
        if (model) {
            return true;
        }
        if (!engine) {
            return false;
        }
        model = new KNSCore::ItemsModel(engine, q);

        q->connect(engine, &Engine::signalProvidersLoaded, engine, &Engine::reloadEntries);
        // Entries have been fetched and should be shown:
        q->connect(engine, &Engine::signalEntriesLoaded, model, [this](const KNSCore::Entry::List &entries) {
            model->slotEntriesLoaded(entries);
        });
        q->connect(engine, &Engine::entryEvent, model, [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
            if (event == KNSCore::Entry::DetailsLoadedEvent && engine->filter() != KNSCore::Provider::Installed
                && engine->filter() != KNSCore::Provider::Updates) {
                model->slotEntriesLoaded(KNSCore::Entry::List{entry});
            }
        });

        // Check if we need intermediate states
        q->connect(engine, &Engine::entryEvent, q, [this](const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event) {
            onEntryEvent(entry, event);
        });
        q->connect(engine, &Engine::signalResetView, model, &KNSCore::ItemsModel::clearEntries);

        q->connect(model, &KNSCore::ItemsModel::loadPreview, engine, &Engine::loadPreview);
        q->connect(engine, &Engine::entryPreviewLoaded, model, &KNSCore::ItemsModel::slotEntryPreviewLoaded);

        q->connect(model, &KNSCore::ItemsModel::rowsInserted, q, &ItemsModel::rowsInserted);
        q->connect(model, &KNSCore::ItemsModel::rowsRemoved, q, &ItemsModel::rowsRemoved);
        q->connect(model, &KNSCore::ItemsModel::dataChanged, q, &ItemsModel::dataChanged);
        q->connect(model, &KNSCore::ItemsModel::modelAboutToBeReset, q, &ItemsModel::modelAboutToBeReset);
        q->connect(model, &KNSCore::ItemsModel::modelReset, q, &ItemsModel::modelReset);
        return true;
    }

    void onEntryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event)
    {
        if (event == KNSCore::Entry::StatusChangedEvent) {
            model->slotEntryChanged(entry);
            Q_EMIT q->entryChanged(entry);

            // If we update/uninstall an entry we have to update the UI, see BUG: 425135
            if (engine->filter() == KNSCore::Provider::Updates && entry.status() != KNSCore::Entry::Updateable && entry.status() != KNSCore::Entry::Updating) {
                model->removeEntry(entry);
            } else if (engine->filter() == KNSCore::Provider::Installed && entry.status() == KNSCore::Entry::Deleted) {
                model->removeEntry(entry);
            }
        }

        if (event == KNSCore::Entry::DetailsLoadedEvent) {
            model->slotEntryChanged(entry);
            Q_EMIT q->entryChanged(entry);
        }
    }
};

ItemsModel::ItemsModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new ItemsModelPrivate(this))
{
}

ItemsModel::~ItemsModel() = default;

QHash<int, QByteArray> ItemsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = QHash<int, QByteArray>{
        {Qt::DisplayRole, "display"},
        {NameRole, "name"},
        {UniqueIdRole, "uniqueId"},
        {CategoryRole, "category"},
        {HomepageRole, "homepage"},
        {AuthorRole, "author"},
        {LicenseRole, "license"},
        {ShortSummaryRole, "shortSummary"},
        {SummaryRole, "summary"},
        {ChangelogRole, "changelog"},
        {VersionRole, "version"},
        {ReleaseDateRole, "releaseDate"},
        {UpdateVersionRole, "updateVersion"},
        {UpdateReleaseDateRole, "updateReleaseDate"},
        {PayloadRole, "payload"},
        {Qt::DecorationRole, "decoration"},
        {PreviewsSmallRole, "previewsSmall"},
        {PreviewsRole, "previews"},
        {InstalledFilesRole, "installedFiles"},
        {UnInstalledFilesRole, "uninstalledFiles"},
        {RatingRole, "rating"},
        {NumberOfCommentsRole, "numberOfComments"},
        {DownloadCountRole, "downloadCount"},
        {NumberFansRole, "numberFans"},
        {NumberKnowledgebaseEntriesRole, "numberKnowledgebaseEntries"},
        {KnowledgebaseLinkRole, "knowledgebaseLink"},
        {DownloadLinksRole, "downloadLinks"},
        {DonationLinkRole, "donationLink"},
        {ProviderIdRole, "providerId"},
        {SourceRole, "source"},
        {EntryRole, "entry"},
    };
    return roles;
}

int ItemsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    if (d->initModel()) {
        return d->model->rowCount(QModelIndex());
    }
    return 0;
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && d->initModel()) {
        KNSCore::Entry entry = d->model->data(d->model->index(index.row()), Qt::UserRole).value<KNSCore::Entry>();
        switch (role) {
        case NameRole:
        case Qt::DisplayRole:
            return entry.name();
        case EntryRole:
            return QVariant::fromValue(entry);
        case UniqueIdRole:
            return entry.uniqueId();
        case CategoryRole:
            return entry.category();
        case HomepageRole:
            return entry.homepage();
            break;
        case AuthorRole: {
            KNSCore::Author author = entry.author();
            QVariantMap returnAuthor;
            returnAuthor[QStringLiteral("id")] = author.id();
            returnAuthor[QStringLiteral("name")] = author.name();
            returnAuthor[QStringLiteral("email")] = author.email();
            returnAuthor[QStringLiteral("homepage")] = author.homepage();
            returnAuthor[QStringLiteral("jabber")] = author.jabber();
            returnAuthor[QStringLiteral("avatarUrl")] = author.avatarUrl();
            returnAuthor[QStringLiteral("description")] = author.description();
            return returnAuthor;
        } break;
        case LicenseRole:
            return entry.license();
        case ShortSummaryRole:
            return entry.shortSummary();
        case SummaryRole:
            return entry.summary();
        case ChangelogRole:
            return entry.changelog();
        case VersionRole:
            return entry.version();
        case ReleaseDateRole:
            return entry.releaseDate();
        case UpdateVersionRole:
            return entry.updateVersion();
        case UpdateReleaseDateRole:
            return entry.updateReleaseDate();
        case PayloadRole:
            return entry.payload();
        case Qt::DecorationRole:
            return entry.previewUrl(KNSCore::Entry::PreviewSmall1);
        case PreviewsSmallRole: {
            QStringList previews;
            previews << entry.previewUrl(KNSCore::Entry::PreviewSmall1);
            previews << entry.previewUrl(KNSCore::Entry::PreviewSmall2);
            previews << entry.previewUrl(KNSCore::Entry::PreviewSmall3);
            while (!previews.isEmpty() && previews.last().isEmpty()) {
                previews.takeLast();
            }
            return previews;
        }
        case PreviewsRole: {
            QStringList previews;
            previews << entry.previewUrl(KNSCore::Entry::PreviewBig1);
            previews << entry.previewUrl(KNSCore::Entry::PreviewBig2);
            previews << entry.previewUrl(KNSCore::Entry::PreviewBig3);
            while (!previews.isEmpty() && previews.last().isEmpty()) {
                previews.takeLast();
            }
            return previews;
        }
        case InstalledFilesRole:
            return entry.installedFiles();
        case UnInstalledFilesRole:
            return entry.uninstalledFiles();
        case RatingRole:
            return entry.rating();
        case NumberOfCommentsRole:
            return entry.numberOfComments();
        case DownloadCountRole:
            return entry.downloadCount();
        case NumberFansRole:
            return entry.numberFans();
        case NumberKnowledgebaseEntriesRole:
            return entry.numberKnowledgebaseEntries();
        case KnowledgebaseLinkRole:
            return entry.knowledgebaseLink();
        case DownloadLinksRole: {
            // This would be good to cache... but it also needs marking as dirty, somehow...
            const QList<KNSCore::Entry::DownloadLinkInformation> dllinks = entry.downloadLinkInformationList();
            QVariantList list;
            for (const KNSCore::Entry::DownloadLinkInformation &link : dllinks) {
                list.append(QVariant::fromValue(DownloadLinkInfo(link)));
            }
            if (list.isEmpty() && !entry.payload().isEmpty()) {
                KNSCore::Entry::DownloadLinkInformation data;
                data.descriptionLink = entry.payload();
                list.append(QVariant::fromValue(DownloadLinkInfo(data)));
            }
            return QVariant::fromValue(list);
        }
        case DonationLinkRole:
            return entry.donationLink();
        case ProviderIdRole:
            return entry.providerId();
        case SourceRole: {
            KNSCore::Entry::Source src = entry.source();
            switch (src) {
            case KNSCore::Entry::Cache:
                return QStringLiteral("Cache");
            case KNSCore::Entry::Online:
                return QStringLiteral("Online");
            case KNSCore::Entry::Registry:
                return QStringLiteral("Registry");
            default:
                return QStringLiteral("Unknown source - shouldn't be possible");
            }
        }
        case CommentsModelRole: {
            KNSCore::CommentsModel *commentsModel{nullptr};
            if (!d->commentsModels.contains(entry.uniqueId())) {
                commentsModel = new KNSCore::CommentsModel(d->engine);
                commentsModel->setEntry(entry);
                d->commentsModels[entry.uniqueId()] = commentsModel;
            } else {
                commentsModel = d->commentsModels[entry.uniqueId()];
            }
            return QVariant::fromValue(commentsModel);
        }
        default:
            return QStringLiteral("Unknown role");
        }
    }
    return QVariant();
}

bool ItemsModel::canFetchMore(const QModelIndex &parent) const
{
    return !parent.isValid() && d->engine && d->engine->categoriesMetadata().count() > 0;
}

void ItemsModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid() || !d->engine) {
        return;
    }
    d->engine->requestMoreData();
}

Engine *ItemsModel::engine() const
{
    return d->engine;
}

void ItemsModel::setEngine(Engine *newEngine)
{
    if (d->engine != newEngine) {
        beginResetModel();
        d->engine = newEngine;
        if (d->model) {
            d->model->deleteLater();
            d->model = nullptr;
        }
        Q_EMIT engineChanged();
        endResetModel();
    }
}

int ItemsModel::indexOfEntryId(const QString &providerId, const QString &entryId)
{
    int idx{-1};
    if (d->engine && d->model) {
        for (int i = 0; i < rowCount(); ++i) {
            KNSCore::Entry testEntry = d->model->data(d->model->index(i), Qt::UserRole).value<KNSCore::Entry>();
            if (providerId == testEntry.providerId() && entryId == testEntry.uniqueId()) {
                idx = i;
                break;
            }
        }
    }
    return idx;
}

#include "moc_quickitemsmodel.cpp"
