/*
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ATTICAPROVIDER_P_H
#define KNEWSTUFF3_ATTICAPROVIDER_P_H

#include <QPointer>
#include <QSet>

#include <attica/content.h>
#include <attica/provider.h>
#include <attica/providermanager.h>

#include "provider.h"

namespace Attica
{
class BaseJob;
}

namespace KNSCore
{
/**
 * @short KNewStuff Attica Provider class.
 *
 * This class provides accessors for the provider object.
 * It should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * websites that implement the Open Collaboration Services.
 *
 * @author Frederik Gladhorn <gladhorn@kde.org>
 *
 * @internal
 */
class AtticaProvider : public Provider
{
    Q_OBJECT
public:
    explicit AtticaProvider(const QStringList &categories, const QString &additionalAgentInformation);
    AtticaProvider(const Attica::Provider &provider, const QStringList &categories, const QString &additionalAgentInformation);

    QString id() const override;

    /**
     * set the provider data xml, to initialize the provider
     */
    bool setProviderXML(const QDomElement &xmldata) override;

    bool isInitialized() const override;
    void setCachedEntries(const KNSCore::Entry::List &cachedEntries) override;

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override;
    void loadEntryDetails(const KNSCore::Entry &entry) override;
    void loadPayloadLink(const Entry &entry, int linkId) override;

    void loadComments(const KNSCore::Entry &entry, int commentsPerPage, int page) override;
    void loadPerson(const QString &username) override;
    void loadBasics() override;

    bool userCanVote() override
    {
        return true;
    }
    void vote(const Entry &entry, uint rating) override;

    bool userCanBecomeFan() override
    {
        return true;
    }
    void becomeFan(const Entry &entry) override;

    Attica::Provider *provider()
    {
        return &m_provider;
    }

private Q_SLOTS:
    void providerLoaded(const Attica::Provider &provider);
    void listOfCategoriesLoaded(Attica::BaseJob *);
    void categoryContentsLoaded(Attica::BaseJob *job);
    void downloadItemLoaded(Attica::BaseJob *job);
    void accountBalanceLoaded(Attica::BaseJob *job);
    void onAuthenticationCredentialsMissing(const Attica::Provider &);
    void votingFinished(Attica::BaseJob *);
    void becomeFanFinished(Attica::BaseJob *job);
    void detailsLoaded(Attica::BaseJob *job);
    void loadedComments(Attica::BaseJob *job);
    void loadedPerson(Attica::BaseJob *job);
    void loadedConfig(Attica::BaseJob *job);

private:
    void checkForUpdates();
    Entry::List installedEntries() const;
    bool jobSuccess(Attica::BaseJob *job);

    Attica::Provider::SortMode atticaSortMode(SortMode sortMode);

    Entry entryFromAtticaContent(const Attica::Content &);

    // the attica categories we are interested in (e.g. Wallpaper, Application, Vocabulary File...)
    QMultiHash<QString, Attica::Category> mCategoryMap;

    Attica::ProviderManager m_providerManager;
    Attica::Provider m_provider;

    KNSCore::Entry::List mCachedEntries;
    QHash<QString, Attica::Content> mCachedContent;

    // Associate job and entry, this is needed when fetching
    // download links or the account balance in order to continue
    // when the result is there.
    QHash<Attica::BaseJob *, QPair<Entry, int>> mDownloadLinkJobs;

    // keep track of the current request
    QPointer<Attica::BaseJob> mEntryJob;
    Provider::SearchRequest mCurrentRequest;

    QSet<Attica::BaseJob *> m_updateJobs;

    bool mInitialized;
    QString m_providerId;

    Q_DISABLE_COPY(AtticaProvider)
};

}

#endif
