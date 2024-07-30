/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_PROVIDER_P_H
#define KNEWSTUFF3_PROVIDER_P_H

#include <QDebug>
#include <QList>
#include <QString>
#include <QUrl>

#include <memory>

#include "entry.h"
#include "errorcode.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{
class ProviderPrivate;
struct Comment;
/**
 * @short KNewStuff Base Provider class.
 *
 * This class provides accessors for the provider object.
 * It should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * static website providers.
 *
 * @author Jeremy Whiting <jpwhiting@kde.org>
 */
class KNEWSTUFFCORE_EXPORT Provider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY basicsLoaded)
    Q_PROPERTY(QUrl website READ website WRITE setWebsite NOTIFY basicsLoaded)
    Q_PROPERTY(QUrl host READ host WRITE setHost NOTIFY basicsLoaded)
    Q_PROPERTY(QString contactEmail READ contactEmail WRITE setContactEmail NOTIFY basicsLoaded)
    Q_PROPERTY(bool supportsSsl READ supportsSsl WRITE setSupportsSsl NOTIFY basicsLoaded)
public:
    typedef QList<Provider *> List;

    enum SortMode {
        Newest,
        Alphabetical,
        Rating,
        Downloads,
    };
    Q_ENUM(SortMode)

    enum Filter {
        None,
        Installed,
        Updates,
        ExactEntryId,
    };
    Q_ENUM(Filter)

    /**
     * used to keep track of a search
     */
    struct SearchRequest {
        SortMode sortMode;
        Filter filter;
        QString searchTerm;
        QStringList categories;
        int page;
        int pageSize;

        SearchRequest(SortMode sortMode_ = Downloads,
                      Filter filter_ = None,
                      const QString &searchTerm_ = QString(),
                      const QStringList &categories_ = QStringList(),
                      int page_ = -1,
                      int pageSize_ = 20)
            : sortMode(sortMode_)
            , filter(filter_)
            , searchTerm(searchTerm_)
            , categories(categories_)
            , page(page_)
            , pageSize(pageSize_)
        {
        }

        QString hashForRequest() const;
        bool operator==(const SearchRequest &other) const
        {
            return sortMode == other.sortMode && filter == other.filter && searchTerm == other.searchTerm && categories == other.categories
                && page == other.page && pageSize == other.pageSize;
        }
    };

    /**
     * Describes a category: id/name/disaplayName
     */
    struct CategoryMetadata {
        QString id;
        QString name;
        QString displayName;
    };

    /**
     * @brief The SearchPresetTypes enum
     * the preset type enum is a helper to identify the kind of label and icon
     * the search preset should have if none are found.
     * @since 5.83
     */
    enum SearchPresetTypes {
        NoPresetType = 0,
        GoBack, ///< preset representing the previous search.
        Root, ///< preset indicating a root directory.
        Start, ///< preset indicating the first entry.
        Popular, ///< preset indicating popular items.
        Featured, ///< preset for featured items.
        Recommended, ///< preset for recommended. This may be customized by the server per user.
        Shelf, ///< preset indicating previously acquired items.
        Subscription, ///< preset indicating items that the user is subscribed to.
        New, ///< preset indicating new items.
        FolderUp, ///< preset indicating going up in the search result hierarchy.
        AllEntries, ///< preset indicating all possible entries, such as a crawlable list. Might be intense to load.
    };
    /**
     * Describes a search request that may come from the provider.
     * This is used by the OPDS provider to handle the different urls.
     * @since 5.83
     */
    struct SearchPreset {
        SearchRequest request;
        QString displayName;
        QString iconName;
        SearchPresetTypes type;
        QString providerId; // not all providers can handle all search requests.
    };

    /**
     * Constructor.
     */
    Provider();

    /**
     * Destructor.
     */
    ~Provider() override;

    /**
     * A unique Id for this provider (the url in most cases)
     */
    virtual QString id() const = 0;

    /**
     * Set the provider data xml, to initialize the provider.
     * The Provider needs to have it's ID set in this function and cannot change it from there on.
     */
    virtual bool setProviderXML(const QDomElement &xmldata) = 0;

    virtual bool isInitialized() const = 0;

    virtual void setCachedEntries(const KNSCore::Entry::List &cachedEntries) = 0;

    /**
     * Retrieves the common name of the provider.
     *
     * @return provider name
     */
    virtual QString name() const;

    /**
     * Retrieves the icon URL for this provider.
     *
     * @return icon URL
     */
    virtual QUrl icon() const; // FIXME use QIcon::fromTheme or pixmap?

    /**
     * load the given search and return given page
     * @param sortMode string to select the order in which the results are presented
     * @param searchstring string to search with
     * @param page         page number to load
     *
     * Note: the engine connects to loadingFinished() signal to get the result
     */
    virtual void loadEntries(const KNSCore::Provider::SearchRequest &request) = 0;
    virtual void loadEntryDetails(const KNSCore::Entry &)
    {
    }
    virtual void loadPayloadLink(const Entry &entry, int linkId) = 0;
    /**
     * Request a loading of comments from this provider. The engine listens to the
     * commentsLoaded() signal for the result
     *
     * @note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support comments.
     *
     * @see commentsLoaded(const QList<shared_ptr<KNSCore::Comment>> comments)
     * @since 5.63
     */
    virtual void loadComments(const KNSCore::Entry &, int /*commentsPerPage*/, int /*page*/)
    {
    }

    /**
     * Request loading of the details for a specific person with the given username.
     * The engine listens to the personLoaded() for the result
     *
     * @note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support comments.
     *
     * @since 5.63
     */
    virtual void loadPerson(const QString & /*username*/)
    {
    }

    /**
     * Request loading of the basic information for this provider. The engine listens
     * to the basicsLoaded() signal for the result, which is also the signal the respective
     * properties listen to.
     *
     * This is fired automatically on the first attempt to read one of the properties
     * which contain this basic information, and you will not need to call it as a user
     * of the class (just listen to the properties, which will update when the information
     * has been fetched).
     *
     * @note Implementation detail: All subclasses should connect to this signal
     * and point it at a slot which does the actual work, if they support fetching
     * this basic information (if the information is set during construction, you will
     * not need to worry about this).
     *
     * @see version()
     * @see website()
     * @see host();
     * @see contactEmail()
     * @see supportsSsl()
     * @since 5.85
     */
    virtual void loadBasics()
    {
    }

    /**
     * @since 5.85
     */
    QString version() const;
    /**
     * @since 5.85
     */
    void setVersion(const QString &version);
    /**
     * @since 5.85
     */
    QUrl website() const;
    /**
     * @since 5.85
     */
    void setWebsite(const QUrl &website);
    /**
     * @since 5.85
     */
    QUrl host() const;
    /**
     * @param host The host used for this provider
     * @since 5.85
     */
    void setHost(const QUrl &host);
    /**
     * The general contact email for this provider
     * @return The general contact email for this provider
     * @since 5.85
     */
    QString contactEmail() const;
    /**
     * Sets the general contact email address for this provider
     * @param contactEmail The general contact email for this provider
     * @since 5.85
     */
    void setContactEmail(const QString &contactEmail);
    /**
     * Whether or not the provider supports SSL connections
     * @return True if the server supports SSL connections, false if not
     * @since 5.85
     */
    bool supportsSsl() const;
    /**
     * Set whether or not the provider supports SSL connections
     * @param supportsSsl True if the server supports SSL connections, false if not
     * @since 5.85
     */
    void setSupportsSsl(bool supportsSsl);

    virtual bool userCanVote()
    {
        return false;
    }
    virtual void vote(const Entry & /*entry*/, uint /*rating*/)
    {
    }

    virtual bool userCanBecomeFan()
    {
        return false;
    }
    virtual void becomeFan(const Entry & /*entry*/)
    {
    }

    /**
     * Set the tag filter used for entries by this provider
     * @param tagFilter The new list of filters
     * @see Engine::setTagFilter(QStringList)
     * @since 5.51
     */
    void setTagFilter(const QStringList &tagFilter);
    /**
     * The tag filter used for downloads by this provider
     * @return The list of filters
     * @see Engine::setTagFilter(QStringList)
     * @since 5.51
     */
    QStringList tagFilter() const;
    /**
     * Set the tag filter used for download items by this provider
     * @param downloadTagFilter The new list of filters
     * @see Engine::setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    void setDownloadTagFilter(const QStringList &downloadTagFilter);
    /**
     * The tag filter used for downloads by this provider
     * @return The list of filters
     * @see Engine::setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    QStringList downloadTagFilter() const;

Q_SIGNALS:
    void providerInitialized(KNSCore::Provider *);

    void loadingFinished(const KNSCore::Provider::SearchRequest &, const KNSCore::Entry::List &);
    void loadingFailed(const KNSCore::Provider::SearchRequest &);

    void entryDetailsLoaded(const KNSCore::Entry &);
    void payloadLinkLoaded(const KNSCore::Entry &);
    /**
     * Fired when new comments have been loaded
     * @param comments The list of newly loaded comments, in a depth-first order
     * @since 5.63
     */
    void commentsLoaded(const QList<std::shared_ptr<KNSCore::Comment>> comments);
    /**
     * Fired when the details of a person have been loaded
     * @param author The person we've just loaded data for
     * @since 5.63
     */
    void personLoaded(const std::shared_ptr<KNSCore::Author> author);
    /**
     * Fired when the provider's basic information has been fetched and updated
     * @since 5.85
     */
    void basicsLoaded();

    /**
     * Fires when the provider has loaded search presets. These represent interesting
     * searches for the user, such as recommendations.
     * @since 5.83
     */
    void searchPresetsLoaded(const QList<KNSCore::Provider::SearchPreset> &presets);

    void signalInformation(const QString &);
    void signalError(const QString &);
    void signalErrorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);

    void categoriesMetadataLoded(const QList<KNSCore::Provider::CategoryMetadata> &categories);

protected:
    void setName(const QString &name);
    void setIcon(const QUrl &icon);

private:
    const std::unique_ptr<ProviderPrivate> d;
    Q_DISABLE_COPY(Provider)
};

KNEWSTUFFCORE_EXPORT QDebug operator<<(QDebug, const Provider::SearchRequest &);
}

#endif
