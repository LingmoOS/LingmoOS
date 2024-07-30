/*
    knewstuff3/entry.h.
    SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2003-2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENTRY
#define KNEWSTUFF3_ENTRY

#include <QDate>
#include <QImage>
#include <QString>
#include <QUrl>

#include "author.h"
#include "knewstuffcore_export.h"

class testEntry;
class QDomElement;
class QXmlStreamReader;

namespace KNSCore
{
static const int PreviewWidth = 96;
static const int PreviewHeight = 72;
class EntryPrivate;

/**
 function to remove bb code formatting that opendesktop sends
 */
KNEWSTUFFCORE_EXPORT QString replaceBBCode(const QString &unformattedText);

/**
 * @short KNewStuff data entry container.
 *
 * This class provides accessor methods to the data objects
 * as used by KNewStuff.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Jeremy Whiting (jpwhiting@kde.org)
 */
class KNEWSTUFFCORE_EXPORT Entry
{
    Q_GADGET
public:
    typedef QList<Entry> List;
    Q_PROPERTY(QString providerId READ providerId)
    Q_PROPERTY(QString uniqueId READ uniqueId)
    Q_PROPERTY(KNSCore::Entry::Status status READ status)
    Q_PROPERTY(KNSCore::Entry::EntryType entryType READ entryType)

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(KNSCore::Author author READ author)
    Q_PROPERTY(QString shortSummary READ shortSummary)
    Q_PROPERTY(QString summary READ summary)
    // TODO  Q_PROPERTY(QString previews READ previews)
    Q_PROPERTY(QUrl homepage READ homepage)
    Q_PROPERTY(QString donationLink READ donationLink)
    Q_PROPERTY(int numberOfComments READ numberOfComments)
    Q_PROPERTY(int rating READ rating)
    Q_PROPERTY(int downloadCount READ downloadCount)
    Q_PROPERTY(QList<KNSCore::Entry::DownloadLinkInformation> downloadLinks READ downloadLinkInformationList)

    /**
     * Status of the entry. An entry will be downloadable from the provider's
     * site prior to the download. Once downloaded and installed, it will
     * be either installed or updateable, implying an out-of-date
     * installation. Finally, the entry can be deleted and hence show up as
     * downloadable again.
     * Entries not taking part in this cycle, for example those in upload,
     * have an invalid status.
     */
    enum Status {
        Invalid,
        Downloadable,
        Installed,
        Updateable,
        Deleted,
        Installing,
        Updating,
    };
    Q_ENUM(Status)

    /**
     * Source of the entry, A entry's data is coming from either cache, or an online provider
     * this helps the engine know which data to use when merging cached entries with online
     * entry data
     */
    enum Source {
        Cache,
        Online,
        Registry,
    };

    enum PreviewType {
        PreviewSmall1,
        PreviewSmall2,
        PreviewSmall3,
        PreviewBig1,
        PreviewBig2,
        PreviewBig3,
    };

    struct DownloadLinkInformation {
        QString name; // Displayed name.
        QString priceAmount; // Price formatted as a string.
        QString distributionType; // OCS Distribution Type, this is for which OS the file is useful.
        QString descriptionLink; // Link to intermediary description.
        int id; // Unique integer representing the download number in the list.
        bool isDownloadtypeLink;
        quint64 size = 0; // Size in kilobytes.
        QStringList tags; // variety of tags that can represent mimetype or url location.
    };

    enum EntryEvent {
        UnknownEvent = 0, ///< A generic event, not generally used
        StatusChangedEvent = 1, ///< Used when an event's status is set (use Entry::status() to get the new status)
        AdoptedEvent = 2, ///< Used when an entry has been successfully adopted (use this to determine whether a call to Engine::adoptEntry() succeeded)
        DetailsLoadedEvent = 3, ///< Used when more details have been added to an existing entry (such as the full description), and the UI should be updated
    };
    Q_ENUM(EntryEvent)

    /**
     * Represents whether the current entry is an actual catalog entry,
     * or an entry that represents a set of entries.
     * @since 5.83
     */
    enum EntryType {
        CatalogEntry = 0, ///< These are the main entries that KNewStuff can get the details about and download links for.
        GroupEntry ///< these are entries whose payload is another feed. Currently only used by the OPDS provider.
    };
    Q_ENUM(EntryType)

    /**
     * Constructor.
     */
    Entry();

    Entry(const Entry &other);
    Entry &operator=(const Entry &other);

    bool operator==(const Entry &other) const;
    bool operator<(const Entry &other) const;

    /**
     * Destructor.
     */
    ~Entry();

    bool isValid() const;

    /**
     * Sets the name for this data object.
     */
    void setName(const QString &name);

    /**
     * Retrieve the name of the data object.
     *
     * @return object name (potentially translated)
     */
    QString name() const;

    /**
     * Set the object's unique ID. This must be unique to the provider.
     *
     * @param id The unique ID of this entry as unique to this provider
     * @see KNSCore::Provider
     */
    void setUniqueId(const QString &id);
    /**
     * Get the object's unique ID. This will be unique to the provider.
     * This is not intended as user-facing information - though it can
     * be useful for certain purposes, this is supposed to only be used
     * for keeping track of the entry.
     *
     * @return The unique ID of this entry
     */
    QString uniqueId() const;

    /**
     * Sets the data category, e.g. "KWin Scripts" or "Plasma Theme".
     */
    void setCategory(const QString &category);

    /**
     * Retrieve the category of the data object. This is the category's
     * name or ID (as opposed to displayName).
     *
     * @see KNSCore::Provider::CategoryMetadata
     * @see KNSCore::Engine::categories()
     * @return object category
     */
    QString category() const;

    /**
     * Set a link to a website containing information about this entry
     *
     * @param page The URL representing the entry's website
     */
    void setHomepage(const QUrl &page);
    /**
     * A link to a website containing information about this entry
     *
     * @return The URL representing the entry's website
     */
    QUrl homepage() const;

    /**
     * Sets the author of the object.
     */
    void setAuthor(const Author &author);

    /**
     * Retrieve the author of the object.
     *
     * @return object author
     */
    Author author() const;

    /**
     * Sets the license (abbreviation) applicable to the object.
     */
    void setLicense(const QString &license);

    /**
     * Retrieve the license name of the object.
     *
     * @return object license
     */
    QString license() const;

    /**
     * Sets a description (which can potentially be very long)
     */
    void setSummary(const QString &summary);

    /**
     * Retrieve a short description of what the object is all about (should be very short)
     *
     * @return object license
     */
    QString shortSummary() const;

    /**
     * Sets a short description of what the object is all about (should be very short)
     */
    void setShortSummary(const QString &summary);

    /**
     * Retrieve a (potentially very long) description of the object.
     *
     * @return object description
     */
    QString summary() const;

    /**
     * The user written changelog
     */
    void setChangelog(const QString &changelog);
    QString changelog() const;

    /**
     * Sets the version number.
     */
    void setVersion(const QString &version);

    /**
     * Retrieve the version string of the object.
     *
     * @return object version
     */
    QString version() const;

    /**
     * Sets the release date.
     */
    void setReleaseDate(const QDate &releasedate);

    /**
     * Retrieve the date of the object's publication.
     *
     * @return object release date
     */
    QDate releaseDate() const;

    /**
     * Sets the version number that is available as update.
     */
    void setUpdateVersion(const QString &version);

    /**
     * Retrieve the version string of the object that is available as update.
     *
     * @return object version
     */
    QString updateVersion() const;

    /**
     * Sets the release date that is available as update.
     */
    void setUpdateReleaseDate(const QDate &releasedate);

    /**
     * Retrieve the date of the newer version that is available as update.
     *
     * @return object release date
     */
    QDate updateReleaseDate() const;

    /**
     * Sets the object's file.
     */
    void setPayload(const QString &url);

    /**
     * Retrieve the file name of the object.
     *
     * @return object filename
     */
    QString payload() const;

    /**
     * Sets the object's preview file, if available. This should be a
     * picture file.
     */
    void setPreviewUrl(const QString &url, PreviewType type = PreviewSmall1);

    /**
     * Retrieve the file name of an image containing a preview of the object.
     *
     * @return object preview filename
     */
    QString previewUrl(PreviewType type = PreviewSmall1) const;

    /**
     * This will not be loaded automatically, instead use Engine to load the actual images.
     */
    QImage previewImage(PreviewType type = PreviewSmall1) const;
    void setPreviewImage(const QImage &image, PreviewType type = PreviewSmall1);

    /**
     * Set the files that have been installed by the install command.
     * @param files local file names
     */
    void setInstalledFiles(const QStringList &files);

    /**
     * Retrieve the locally installed files.
     * @return file names
     */
    QStringList installedFiles() const;

    /**
     * Retrieve the locally uninstalled files.
     * @return file names
     * @since 4.1
     */
    QStringList uninstalledFiles() const;

    /**
     * Sets the rating between 0 (worst) and 100 (best).
     *
     * @internal
     */
    void setRating(int rating);

    /**
     * Retrieve the rating for the object, which has been determined by its
     * users and thus might change over time.
     *
     * @return object rating
     */
    int rating() const;

    /**
     * Sets the number of comments in the asset
     *
     * @internal
     */
    void setNumberOfComments(int comments);

    /**
     * @returns the number of comments against the asset
     */
    int numberOfComments() const;

    /**
     * Sets the number of downloads.
     *
     * @internal
     */
    void setDownloadCount(int downloads);

    /**
     * Retrieve the download count for the object, which has been determined
     * by its hosting sites and thus might change over time.
     *
     * @return object download count
     */
    int downloadCount() const;

    /**
     * How many people have marked themselves as fans of this entry
     *
     * @return The number of fans this entry has
     * @see KNSCore::Engine::becomeFan(const Entry& entry)
     */
    int numberFans() const;
    /**
     * Sets how many people are fans.
     * Note: This is purely informational. To become a fan, call the
     * KNSCore::Engine::becomeFan function.
     *
     * @param fans The number of fans this entry has
     * @see KNSCore::Engine::becomeFan(const Entry& entry)
     */
    void setNumberFans(int fans);

    /**
     * The number of entries in the knowledgebase for this entry
     * @return The number of knowledgebase entries
     */
    int numberKnowledgebaseEntries() const;
    /**
     * Set the number of knowledgebase entries for this entry
     * @param num The number of entries
     */
    void setNumberKnowledgebaseEntries(int num);
    /**
     * The link for the knowledgebase for this entry.
     * @return A string version of the URL for the knowledgebase
     */
    QString knowledgebaseLink() const;
    /**
     * Set the link for the knowledgebase.
     * Note: This is not checked for validity, the caller must do this.
     * @param link The string version of the URL for the knowledgebase
     */
    void setKnowledgebaseLink(const QString &link);

    /**
     * The number of available download options for this entry
     * @return The number of download options
     */
    int downloadLinkCount() const;
    /**
     * A list of downloadable data for this entry
     * @return The list of download options
     * @see DownloadLinkInformation
     */
    QList<DownloadLinkInformation> downloadLinkInformationList() const;
    /**
     * Add a new download option to this entry
     * @param info The new download option
     */
    void appendDownloadLinkInformation(const DownloadLinkInformation &info);
    /**
     * Remove all download options from this entry
     */
    void clearDownloadLinkInformation();

    /**
     * A string representing the URL for a website where the user can donate
     * to the author of this entry
     * @return The string version of the URL for the entry's donation website
     */
    QString donationLink() const;
    /**
     * Set a string representation of the URL for the donation website for this entry.
     * Note: This is not checked for validity, the caller must do this.
     * @param link String version of the URL for the entry's donation website
     */
    void setDonationLink(const QString &link);

    /**
     * The set of tags assigned specifically to this content item. This does not include
     * tags for the download links. To get those, you must concatenate the lists yourself.
     * @see downloadLinkInformationList()
     * @see DownloadLinkInformation
     * @see Engine::setTagFilter(QStringList)
     * @since 5.51
     */
    QStringList tags() const;
    /**
     * Set the tags for the content item.
     * @param tags A string list containing the tags for this entry
     * @since 5.51
     */
    void setTags(const QStringList &tags);

    /**
      The id of the provider this entry belongs to
      */
    QString providerId() const;
    void setProviderId(const QString &id);

    /**
      The source of this entry can be Cache, Registry or Online - @see source
      */
    void setSource(Source source);
    Source source() const;

    /**
     * The entry type is either catalog entry, or group entry.
     * @since 5.83
     */
    void setEntryType(EntryType type);
    EntryType entryType() const;

    /**
     * set the xml for the entry
     * parses the xml and sets the private members accordingly
     * used to deserialize data loaded from provider
     *
     * @param xmldata string to load xml data from
     *
     * @returns whether or not setting the values was successful
     *
     * @since 5.36
     */
    bool setEntryXML(QXmlStreamReader &reader);

    /**
     * Sets the entry's status. If no status is set, the default will be
     * \ref Invalid.
     *
     * Note that while this enum is currently found in KNS3::Entry,
     * it will be moved to this class once the binary compatibility
     * lock is lifted for Frameworks 6. For now, you should read any
     * reference to the KNS3::Entry::Status enumerator as KNSCore::Entry::Status
     *
     * @param status New status of the entry
     */
    void setStatus(KNSCore::Entry::Status status);

    /**
     * Retrieves the entry's status.
     *
     * @return Current status of the entry
     */
    KNSCore::Entry::Status status() const;

    /// @internal
    void setEntryDeleted();

private:
    friend class StaticXmlProvider;
    friend class Cache;
    friend class Installation;
    friend class AtticaProvider;
    friend class Transaction;
    friend testEntry;
    KNEWSTUFFCORE_NO_EXPORT void setEntryRequestedId(const QString &id);
    QDomElement entryXML() const;
    bool setEntryXML(const QDomElement &xmldata);
    QExplicitlySharedDataPointer<EntryPrivate> d;
};

inline size_t qHash(const KNSCore::Entry &entry, size_t seed = 0)
{
    return qHash(entry.uniqueId(), seed);
}

KNEWSTUFFCORE_EXPORT QDebug operator<<(QDebug debug, const KNSCore::Entry &entry);
}

Q_DECLARE_TYPEINFO(KNSCore::Entry, Q_RELOCATABLE_TYPE);

#endif
