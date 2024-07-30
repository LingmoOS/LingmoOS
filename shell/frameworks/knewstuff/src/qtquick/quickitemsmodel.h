/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ITEMSMODEL_H
#define ITEMSMODEL_H

#include <QAbstractListModel>

#include "entry.h"
#include "quickengine.h"

#include <memory>

class ItemsModelPrivate;

/**
 * @short A model which shows the contents found in an Engine
 *
 * Use an instance of this model to show the content items represented by the configuration
 * file passed to an engine. The following sample assumes you are using the Engine component,
 * however it is also possible to pass a KNSCore::EngineBase instance created from C++ to this
 * property, if you have specific requirements not covered by the convenience component.
 *
 * Most data in the model is simple, but the DownloadLinks role will return a list of
 * DownloadLinkInfo entries, which you will need to manage in some way.
 *
 * You might also look at NewStuffList, NewStuffItem, and the other items, to see some more
 * detail on what can be done with the data.
 *
 * @see NewStuffList
 * @see NewStuffItem
 * @see NewStuffPage
 * @see NewStuffEntryDetails
 * @see NewStuffEntryComments
 *
 * \code
    import org.kde.newstuff as NewStuff
    Item {
        NewStuff.ItemsModel {
            id: newStuffModel
            engine: newStuffEngine
        }
        NewStuff.Engine {
            id: newStuffEngine
            configFile: "/some/filesystem/location/wallpaper.knsrc"
            onBusyMessageChanged: () => console.log("KNS Message: " + newStuffEngine.busyMessage)
            onErrorCode: (code, message, metadata) => console.log("KNS Error: " + message)
        }
    }
    \endcode
 */
class ItemsModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The NewStuffQuickEngine to show items from
     */
    Q_PROPERTY(Engine *engine READ engine WRITE setEngine NOTIFY engineChanged REQUIRED)
public:
    explicit ItemsModel(QObject *parent = nullptr);
    ~ItemsModel() override;

    enum Roles {
        NameRole = Qt::UserRole + 1,
        UniqueIdRole,
        CategoryRole,
        HomepageRole,
        AuthorRole,
        LicenseRole,
        ShortSummaryRole,
        SummaryRole,
        ChangelogRole,
        VersionRole,
        ReleaseDateRole,
        UpdateVersionRole,
        UpdateReleaseDateRole,
        PayloadRole,
        PreviewsSmallRole, ///@< this will return a list here, rather than be tied so tightly to the remote api
        PreviewsRole, ///@< this will return a list here, rather than be tied so tightly to the remote api
        InstalledFilesRole,
        UnInstalledFilesRole,
        RatingRole,
        NumberOfCommentsRole,
        DownloadCountRole,
        NumberFansRole,
        NumberKnowledgebaseEntriesRole,
        KnowledgebaseLinkRole,
        DownloadLinksRole,
        DonationLinkRole,
        ProviderIdRole,
        SourceRole,
        CommentsModelRole,
        EntryRole,
    };
    Q_ENUM(Roles)

    // The lists in OCS are one-indexed, and that isn't how one usually does things in C++.
    // Consequently, this enum removes what would seem like magic numbers from the code, and
    // makes their meaning more explicit.
    enum LinkId { // TODO KF6 reuse this enum in the transaction, we currently use magic numbers there
        AutoDetectLinkId = -1,
        FirstLinkId = 1,
    };
    Q_ENUM(LinkId)

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    Engine *engine() const;
    void setEngine(Engine *newEngine);
    Q_SIGNAL void engineChanged();

    /**
     * Get the index of an entry based on that entry's unique ID
     * @param providerId The provider inside of which you wish to search for an entry
     * @param entryId The unique ID within the given provider of the entry you want to know the index of
     * @return The index of the entry. In case the entry is not found, -1 is returned
     * @see KNSCore::Entry::uniqueId()
     * @since 5.79
     */
    Q_INVOKABLE int indexOfEntryId(const QString &providerId, const QString &entryId);
    Q_INVOKABLE int indexOfEntry(const KNSCore::Entry &e)
    {
        return indexOfEntryId(e.providerId(), e.uniqueId());
    }

    /**
     * @brief Fired when an entry's data changes
     *
     * @param entry The entry which has changed
     */
    Q_SIGNAL void entryChanged(const KNSCore::Entry &entry);

private:
    const std::unique_ptr<ItemsModelPrivate> d;
};

#endif // ITEMSMODEL_H
