/*
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2007-2010 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ENGINEBASE_H
#define KNEWSTUFF3_ENGINEBASE_H

#include <QHash>
#include <QMetaEnum>
#include <QObject>
#include <QSharedPointer>
#include <QString>

#include "entry.h"
#include "errorcode.h"
#include "provider.h"

#include "knewstuffcore_export.h"

#include <memory>

class KJob;
class EnginePrivate;
class QDomDocument;

namespace Attica
{
class Provider;
}

namespace KNSCore
{
class Cache;
class CommentsModel;
class ResultsStream;
class EngineBasePrivate;
class Installation;

/**
 * KNewStuff engine.
 * An engine keeps track of data which is available locally and remote
 * and offers high-level synchronization calls as well as upload and download
 * primitives using an underlying GHNS protocol.
 *
 * This is a base class for different engine implementations
 */
class KNEWSTUFFCORE_EXPORT EngineBase : public QObject
{
    Q_OBJECT

    /**
     * Text that should be displayed for the adoption button, this defaults to "Use"
     * @since 5.77
     */
    Q_PROPERTY(QString useLabel READ useLabel NOTIFY useLabelChanged)

    /**
     * Whether or not the configuration says that the providers are expected to support uploading.
     * As it stands, this is used to determine whether or not to show the Upload... action where
     * that is displayed (primarily NewStuff.Page).
     * @since 5.85
     */
    Q_PROPERTY(bool uploadEnabled READ uploadEnabled NOTIFY uploadEnabledChanged)

    /**
     * @since 5.85
     */
    Q_PROPERTY(QStringList providerIDs READ providerIDs NOTIFY providersChanged)

    /**
     * @copydoc contentWarningType
     */
    Q_PROPERTY(ContentWarningType contentWarningType READ contentWarningType NOTIFY contentWarningTypeChanged)

public:
    EngineBase(QObject *parent = nullptr);
    ~EngineBase();

    /**
     * List of all available config files. This list will contain no duplicated filenames.
     * The returned file paths are absolute.
     * @since 5.83
     */
    static QStringList availableConfigFiles();

    /**
     * Initializes the engine. This step is application-specific and relies
     * on an external configuration file, which determines all the details
     * about the initialization.
     *
     * @param configfile KNewStuff2 configuration file (*.knsrc)
     * @return \b true if any valid configuration was found, \b false otherwise
     * @see KNS3::DownloadDialog
     */
    virtual bool init(const QString &configfile);

    /**
     * The name as defined by the knsrc file
     * @return The name associated with the engine's configuration file
     * @since 5.63
     */
    QString name() const;

    /**
     * Text that should be displayed for the adoption button, this defaults to i18n("Use")
     * @since 5.77
     */
    QString useLabel() const;

    /**
     * Signal gets emitted when the useLabel property changes
     * @since 5.77
     */
    Q_SIGNAL void useLabelChanged();

    /**
     * Whether or not the configuration says that the providers are expected to support uploading.
     * @return True if the providers are expected to support uploading
     * @since 5.85
     */
    bool uploadEnabled() const;

    /**
     * Fired when the uploadEnabled property changes
     * @since 5.85
     */
    Q_SIGNAL void uploadEnabledChanged();

    /**
     * The list of the server-side names of the categories handled by this
     * engine instance. This corresponds directly to the list of categories
     * in your knsrc file. This is not supposed to be used as user-facing
     * strings - @see categoriesMetadata() for that.
     *
     * @return The categories which this instance of Engine handles
     */
    QStringList categories() const;

    /**
     * Get the entries cache for this engine (note that it may be null if the engine is
     * not yet initialized).
     * @return The cache for this engine (or null if the engine is not initialized)
     * @since 5.74
     */
    QSharedPointer<Cache> cache() const;

    /**
     * The list of metadata for the categories handled by this engine instance.
     * If you wish to show the categories to the user, this is the data to use.
     * The category name is the string used to set categories for the filter,
     * and also what is returned by both categories() and categoriesFilter().
     * The human-readable name is displayName, and the only thing which should
     * be shown to the user.
     *
     * @return The metadata for all categories handled by this engine
     */
    QList<Provider::CategoryMetadata> categoriesMetadata();

    QList<Provider::SearchPreset> searchPresets();

    /**
     * @returns the list of attica (OCS) providers this engine is connected to
     * @since 5.92
     */
    QList<Attica::Provider *> atticaProviders() const;

    /**
     * Set a filter for results, which filters out all entries which do not match
     * the filter, as applied to the tags for the entry. This filters only on the
     * tags specified for the entry itself. To filter the downloadlinks, use
     * setDownloadTagFilter(QStringList).
     *
     * @note The default filter if one is not set from your knsrc file will filter
     * out entries marked as ghns_excluded=1. To retain this when setting a custom
     * filter, add "ghns_excluded!=1" as one of the filters.
     *
     * @note Some tags provided by OCS do not supply a value (and are simply passed
     * as a key). These will be interpreted as having the value 1 for filtering
     * purposes. An example of this might be ghns_excluded, which in reality will
     * generally be passed through ocs as "ghns_excluded" rather than "ghns_excluded=1"
     *
     * @note As tags are metadata, they are provided in the form of adjectives. They
     * are never supplied as action verbs or instructions (as an example, a good tag
     * to suggest that for example a wallpaper is painted would be "painted" as opposed
     * to "paint", and another example might be that an item should be "excluded" as
     * opposed to "exclude").
     *
     * == Examples of use ==
     * Value for tag "tagname" must be exactly "tagdata":
     * tagname==tagdata
     *
     * Value for tag "tagname" must be different from "tagdata":
     * tagname!=tagdata
     *
     * == KNSRC entry ==
     * A tag filter line in a .knsrc file, which is a comma separated list of
     * tag/value pairs, might look like:
     *
     * TagFilter=ghns_excluded!=1,data##mimetype==application/cbr+zip,data##mimetype==application/cbr+rar
     * which would honour the exclusion and filter out anything that does not
     * include a comic book archive in either zip or rar format in one or more
     * of the download items.
     * Notice in particular that there are two data##mimetype entries. Use this
     * for when a tag may have multiple values.
     *
     * TagFilter=application##architecture==x86_64
     * which would not honour the exclusion, and would filter out all entries
     * which do not mark themselves as having a 64bit application binary in at
     * least one download item.
     *
     * The value does not current support wildcards. The list should be considered
     * a binary AND operation (that is, all filter entries must match for the data
     * entry to be included in the return data)
     *
     * @param filter The filter in the form of a list of strings
     * @see setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    void setTagFilter(const QStringList &filter);
    /**
     * Gets the current tag filter list
     * @see setTagFilter(QStringList)
     * @since 5.51
     */
    QStringList tagFilter() const;
    /**
     * Add a single filter entry to the entry tag filter. The filter should be in
     * the same form as the filter lines in the list used by setTagFilter(QStringList)
     * @param filter The filter in the form of a string
     * @see setTagFilter(QStringList)
     * @since 5.51
     */
    void addTagFilter(const QString &filter);
    /**
     * Sets a filter to be applied to the downloads for an entry. The logic is the
     * same as used in setTagFilter(QStringList), but vitally, only one downloadlink
     * is required to match the filter for the list to be valid. If you do not wish
     * to show the others in your client, you must hide them yourself.
     *
     * For an entry to be accepted when a download tag filter is set, it must also
     * be accepted by the entry filter (so, for example, while a list of downloads
     * might be accepted, if the entry has ghns_excluded set, and the default entry
     * filter is set, the entry will still be filtered out).
     *
     * In your knsrc file, set DownloadTagFilter to the filter you wish to apply,
     * using the same logic as described for the entry tagfilter.
     *
     * @param filter The filter in the form of a list of strings
     * @see setTagFilter(QStringList)
     * @since 5.51
     */
    void setDownloadTagFilter(const QStringList &filter);
    /**
     * Gets the current downloadlink tag filter list
     * @see setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    QStringList downloadTagFilter() const;
    /**
     * Add a single filter entry to the download tag filter. The filter should be in
     * the same form as the filter lines in the list used by setDownloadsTagFilter(QStringList)
     * @param filter The filter in the form of a string
     * @see setTagFilter(QStringList)
     * @see setDownloadTagFilter(QStringList)
     * @since 5.51
     */
    void addDownloadTagFilter(const QString &filter);

    /**
     * Whether or not a user is able to vote on the passed entry.
     *
     * @param entry The entry to check votability on
     * @return True if the user is able to vote on the entry
     */
    bool userCanVote(const Entry &entry);
    /**
     * Cast a vote on the passed entry.
     *
     * @param entry The entry to vote on
     * @param rating A number from 0 to 100, 50 being neutral, 0 being most negative and 100 being most positive.
     */
    void vote(const Entry &entry, uint rating);

    /**
     * Whether or not the user is allowed to become a fan of
     * a particular entry.
     * Not all providers (and consequently entries) support the fan functionality
     * and you can use this function to determine this ability.
     * @param entry The entry the user might wish to be a fan of
     * @return Whether or not it is possible for the user to become a fan of that entry
     */
    bool userCanBecomeFan(const Entry &entry);
    /**
     * This will mark the user who is currently authenticated as a fan
     * of the entry passed to the function.
     * @param entry The entry the user wants to be a fan of
     */
    void becomeFan(const Entry &entry);
    // FIXME There is currently no exposed API to remove the fan status

    /**
     * The Provider instance with the passed ID
     *
     * @param providerId The ID of the Provider to fetch
     * @return The Provider with the passed ID, or null if non such Provider exists
     * @since 5.63
     */
    QSharedPointer<Provider> provider(const QString &providerId) const;

    /**
     * Return the first provider in the providers list (usually the default provider)
     * @return The first Provider (or null if the engine is not initialized)
     * @since 5.63
     */
    QSharedPointer<Provider> defaultProvider() const;

    /**
     * The IDs of all providers known by this engine. Use this in combination with
     * provider(const QString&) to iterate over all providers.
     * @return The string IDs of all known providers
     * @since 5.85
     */
    QStringList providerIDs() const;

    /**
     * Whether or not an adoption command exists for this engine
     *
     * @see adoptionCommand(KNSCore::Entry)
     * @return True if an adoption command exists
     */
    bool hasAdoptionCommand() const;

    /**
     * Returns a stream object that will fulfill the @p request.
     *
     * @since 6.0
     */
    ResultsStream *search(const KNSCore::Provider::SearchRequest &request);

    /**
     * @brief The ContentWarningType enum
     * @since 6.1
     */
    enum class ContentWarningType {
        /**
         * Content consists of static assets only
         * Installation should not pose a security risk
         */
        Static,
        /**
         * Content may contain scripts or other executable code
         * Users should be warned to treat it like any other program
         */
        Executables
    };
    Q_ENUM(ContentWarningType)

    /**
     * @brief The level of warning that should be presented to the user
     * @since 6.1
     * @see ContentWarningType
     */
    ContentWarningType contentWarningType() const;

    /**
     * Emitted after the initial config load
     * @since 6.1
     */
    Q_SIGNAL void contentWarningTypeChanged();

Q_SIGNALS:
    /**
     * Indicates a message to be added to the ui's log, or sent to a messagebox
     */
    void signalMessage(const QString &message);

    void signalProvidersLoaded();

    /**
     * Fires in the case of any critical or serious errors, such as network or API problems.
     * @param errorCode Represents the specific type of error which has occurred
     * @param message A human-readable message which can be shown to the end user
     * @param metadata Any additional data which might be hepful to further work out the details of the error (see KNSCore::Entry::ErrorCode for the
     * metadata details)
     * @see KNSCore::Entry::ErrorCode
     * @since 5.53
     */
    void signalErrorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);

    void signalCategoriesMetadataLoded(const QList<Provider::CategoryMetadata> &categories);

    /**
     * Fires when the engine has loaded search presets. These represent interesting
     * searches for the user, such as recommendations.
     * @since 5.83
     */
    void signalSearchPresetsLoaded(const QList<Provider::SearchPreset> &presets);

    /**
     * Fired whenever the list of providers changes
     * @since 5.85
     */
    void providersChanged();

    void loadingProvider();

private:
    // the .knsrc file was loaded
    void slotProviderFileLoaded(const QDomDocument &doc);
    // instead of getting providers from knsrc, use what was configured in ocs systemsettings
    void atticaProviderLoaded(const Attica::Provider &provider);
    // called when a provider is ready to work
    void providerInitialized(KNSCore::Provider *);

    // loading the .knsrc file failed
    void slotProvidersFailed();

    /**
     * load providers from the providersurl in the knsrc file
     * creates providers based on their type and adds them to the list of providers
     */
    void loadProviders();

protected:
    /**
      Add a provider and connect it to the right slots
     */
    virtual void addProvider(QSharedPointer<KNSCore::Provider> provider);
    virtual void updateStatus();

    friend class ResultsStream;
    friend class Transaction;
    Installation *installation() const; // Needed for quick engine
    QList<QSharedPointer<Provider>> providers() const;
    std::unique_ptr<EngineBasePrivate> d;
};

}

#endif
