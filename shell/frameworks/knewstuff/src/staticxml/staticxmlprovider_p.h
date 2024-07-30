/*
    knewstuff3/provider.h
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2009 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2009-2010 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_STATICXMLPROVIDER_P_H
#define KNEWSTUFF3_STATICXMLPROVIDER_P_H

#include "provider.h"
#include <QDomDocument>
#include <QMap>

namespace KNSCore
{
class XmlLoader;

/**
 * @short KNewStuff Base Provider class.
 *
 * This class provides accessors for the provider object.
 * It should not be used directly by the application.
 * This class is the base class and will be instantiated for
 * static website providers.
 *
 * @author Jeremy Whiting <jpwhiting@kde.org>
 *
 * @internal
 */
class StaticXmlProvider : public Provider
{
    Q_OBJECT
public:
    typedef QList<Provider *> List;
    /**
     * Constructor.
     */
    StaticXmlProvider();

    QString id() const override;

    /**
     * set the provider data xml, to initialize the provider
     */
    bool setProviderXML(const QDomElement &xmldata) override;

    bool isInitialized() const override;

    void setCachedEntries(const KNSCore::Entry::List &cachedEntries) override;

    void loadEntries(const KNSCore::Provider::SearchRequest &request) override;
    void loadPayloadLink(const KNSCore::Entry &entry, int) override;

private Q_SLOTS:
    void slotEmitProviderInitialized();
    void slotFeedFileLoaded(const QDomDocument &);
    void slotFeedFailed();

private:
    bool searchIncludesEntry(const Entry &entry) const;
    QUrl downloadUrl(SortMode mode) const;
    Entry::List installedEntries() const;

    // map of download urls to their feed name
    QMap<QString, QUrl> mDownloadUrls;
    QUrl mUploadUrl;
    QUrl mNoUploadUrl;

    // cache of all entries known from this provider so far, mapped by their id
    Entry::List mCachedEntries;
    QMap<Provider::SortMode, XmlLoader *> mFeedLoaders;
    Provider::SearchRequest mCurrentRequest;
    QString mId;
    bool mInitialized;

    Q_DISABLE_COPY(StaticXmlProvider)
};

}

#endif
