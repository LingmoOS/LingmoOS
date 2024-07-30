/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QQmlListProperty>

#include "categoriesmodel.h"
#include "enginebase.h"
#include "entry.h"
#include "errorcode.h"
#include "provider.h"
#include "searchpresetmodel.h"
#include "transaction.h"

class EnginePrivate;

/**
 * KNSCore::EngineBase for interfacing with QML
 *
 * @see ItemsModel
 */
class Engine : public KNSCore::EngineBase
{
    Q_OBJECT
    Q_PROPERTY(QString configFile READ configFile WRITE setConfigFile NOTIFY configFileChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY busyStateChanged)
    Q_PROPERTY(bool needsLazyLoadSpinner READ needsLazyLoadSpinner NOTIFY busyStateChanged)
    Q_PROPERTY(bool hasAdoptionCommand READ hasAdoptionCommand NOTIFY configFileChanged)
    Q_PROPERTY(QString name READ name NOTIFY configFileChanged)
    Q_PROPERTY(bool isValid READ isValid NOTIFY configFileChanged)

    Q_PROPERTY(CategoriesModel *categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QStringList categoriesFilter READ categoriesFilter WRITE setCategoriesFilter RESET resetCategoriesFilter NOTIFY categoriesFilterChanged)
    Q_PROPERTY(KNSCore::Provider::Filter filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(KNSCore::Provider::SortMode sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)
    Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm RESET resetSearchTerm NOTIFY searchTermChanged)
    Q_PROPERTY(SearchPresetModel *searchPresetModel READ searchPresetModel NOTIFY searchPresetModelChanged)

    /**
     * Current state of the engine, the state con contain multiple operations
     * an empty BusyState represents the idle status
     * @since 5.74
     */
    Q_PROPERTY(BusyState busyState READ busyState WRITE setBusyState NOTIFY busyStateChanged)
    Q_PROPERTY(QString busyMessage READ busyMessage NOTIFY busyStateChanged)
public:
    explicit Engine(QObject *parent = nullptr);
    ~Engine() override;

    enum class BusyOperation {
        Initializing = 1,
        LoadingData,
        LoadingPreview,
        InstallingEntry,
    };
    Q_DECLARE_FLAGS(BusyState, BusyOperation)
    Q_ENUM(BusyOperation)

    enum EntryEvent { // TODO KF6 remove in favor of using NewStuff.Entry values
        UnknownEvent = KNSCore::Entry::UnknownEvent,
        StatusChangedEvent = KNSCore::Entry::StatusChangedEvent,
        AdoptedEvent = KNSCore::Entry::AdoptedEvent,
        DetailsLoadedEvent = KNSCore::Entry::DetailsLoadedEvent,
    };
    Q_ENUM(EntryEvent)

    QString configFile() const;
    void setConfigFile(const QString &newFile);
    Q_SIGNAL void configFileChanged();

    Engine::BusyState busyState() const;
    QString busyMessage() const;
    void setBusyState(Engine::BusyState state);

    /**
     * Signal gets emitted when the busy state changes
     * @since 5.74
     */
    Q_SIGNAL void busyStateChanged();

    /**
     * Whether or not the engine is performing its initial loading operations
     * @since 5.65
     */
    bool isLoading() const
    {
        // When installing entries, we don't want to block the UI
        return busyState().toInt() != 0 && ((busyState() & BusyOperation::InstallingEntry) != BusyOperation::InstallingEntry);
    }

    CategoriesModel *categories() const;
    Q_SIGNAL void categoriesChanged();

    QStringList categoriesFilter() const;
    void setCategoriesFilter(const QStringList &newCategoriesFilter);
    Q_INVOKABLE void resetCategoriesFilter()
    {
        setCategoriesFilter(categoriesFilter());
    }
    Q_SIGNAL void categoriesFilterChanged();

    KNSCore::Provider::Filter filter() const;
    void setFilter(KNSCore::Provider::Filter filter);
    Q_SIGNAL void filterChanged();

    KNSCore::Provider::SortMode sortOrder() const;
    void setSortOrder(KNSCore::Provider::SortMode newSortOrder);
    Q_SIGNAL void sortOrderChanged();

    QString searchTerm() const;
    void setSearchTerm(const QString &newSearchTerm);
    Q_INVOKABLE void resetSearchTerm()
    {
        setSearchTerm(QString());
    }
    Q_SIGNAL void searchTermChanged();

    SearchPresetModel *searchPresetModel() const;
    Q_SIGNAL void searchPresetModelChanged();

    Q_INVOKABLE void updateEntryContents(const KNSCore::Entry &entry);
    Q_INVOKABLE KNSCore::Entry __createEntry(const QString &providerId, const QString &entryId)
    {
        KNSCore::Entry e;
        e.setProviderId(providerId);
        e.setUniqueId(entryId);
        return e;
    }

    bool isValid();
    void reloadEntries();

    void loadPreview(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type);

    void addProvider(QSharedPointer<KNSCore::Provider> provider) override;

    /**
     * Adopt an entry using the adoption command. This will also take care of displaying error messages
     * @param entry Entry that should be adopted
     * @see signalErrorCode
     * @see signalEntryEvent
     * @since 5.77
     */
    Q_INVOKABLE void adoptEntry(const KNSCore::Entry &entry);

    /**
     * Installs an entry's payload file. This includes verification, if
     * necessary, as well as decompression and other steps according to the
     * application's *.knsrc file.
     *
     * @param entry Entry to be installed
     *
     * @see signalInstallationFinished
     * @see signalInstallationFailed
     */
    Q_INVOKABLE void install(const KNSCore::Entry &entry, int linkId = 1);

    /**
     * Uninstalls an entry. It reverses the steps which were performed
     * during the installation.
     *
     * @param entry The entry to deinstall
     */
    Q_INVOKABLE void uninstall(const KNSCore::Entry &entry);

    void requestMoreData();

    Q_INVOKABLE void revalidateCacheEntries();
    Q_INVOKABLE void restoreSearch();
    Q_INVOKABLE void storeSearch();
Q_SIGNALS:
    void signalResetView();

    /**
     * This is fired for events related directly to a single Entry instance
     * The intermediate states Updating and Installing are not forwarded. In case you
     * need those you have to listen to the signals of the KNSCore::Engine instance of the engine property.
     *
     * As an example, if you need to know when the status of an entry changes, you might write:
     \code
        function onEntryEvent(entry, event) {
            if (event == NewStuff.Engine.StatusChangedEvent) {
                myModel.ghnsEntryChanged(entry);
            }
        }
     \endcode
     *
     * nb: The above example is also how one would port a handler for the old changedEntries signal
     *
     * @see Entry::EntryEvent for details on which specific event is being notified
     */
    void entryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event);

    /**
     * Fires in the case of any critical or serious errors, such as network or API problems.
     * This forwards the signal from KNSCore::Engine::signalErrorCode, but with QML friendly
     * enumerations.
     * @param errorCode Represents the specific type of error which has occurred
     * @param message A human-readable message which can be shown to the end user
     * @param metadata Any additional data which might be hepful to further work out the details of the error (see KNSCore::Entry::ErrorCode for the
     * metadata details)
     * @see KNSCore::Engine::signalErrorCode
     * @since 5.84
     */
    void errorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);

    void entryPreviewLoaded(const KNSCore::Entry &, KNSCore::Entry::PreviewType);

    void signalEntriesLoaded(const KNSCore::Entry::List &entries); ///@internal
    void signalEntryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event); ///@internal

private:
    bool init(const QString &configfile) override;
    void updateStatus() override;
    bool needsLazyLoadSpinner();
    Q_SIGNAL void signalEntryPreviewLoaded(const KNSCore::Entry &, KNSCore::Entry::PreviewType);
    void registerTransaction(KNSCore::Transaction *transactions);
    void doRequest();
    const std::unique_ptr<EnginePrivate> d;
};

#endif // ENGINE_H
