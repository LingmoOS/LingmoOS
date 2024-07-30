/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KMIMEASSOCIATIONS_H
#define KMIMEASSOCIATIONS_H

#include <QHash>
#include <QSet>
#include <kserviceoffer.h>
class KConfigGroup;
class KServiceFactory;

struct ServiceTypeOffersData {
    QList<KServiceOffer> offers; // service + initial preference + allow as default
    QSet<KService::Ptr> offerSet; // for quick contains() check
    QSet<KService::Ptr> removedOffers; // remember removed offers explicitly
};

class KOfferHash
{
public:
    KOfferHash()
    {
    }
    KOfferHash(const KOfferHash &) = delete;
    KOfferHash &operator=(const KOfferHash &) = delete;
    QList<KServiceOffer> offersFor(const QString &serviceType) const
    {
        auto it = m_serviceTypeData.constFind(serviceType);
        if (it != m_serviceTypeData.cend()) {
            return it.value().offers;
        }
        return QList<KServiceOffer>();
    }
    void addServiceOffer(const QString &serviceType, const KServiceOffer &offer);
    void removeServiceOffer(const QString &serviceType, const KService::Ptr &service);
    bool hasRemovedOffer(const QString &serviceType, const KService::Ptr &service) const;

    const QHash<QString, ServiceTypeOffersData> &serviceTypeData() const
    {
        return m_serviceTypeData;
    }

private:
    QHash<QString, ServiceTypeOffersData> m_serviceTypeData;
};

/**
 * Parse mimeapps.list files and:
 * - modify MIME type associations in the relevant services (using KServiceFactory)
 * - remember preference order specified by user
 */
class KMimeAssociations
{
public:
    explicit KMimeAssociations(KOfferHash &offerHash, KServiceFactory *serviceFactory);

    static QStringList mimeAppsFiles();

    // Read mimeapps.list files
    void parseAllMimeAppsList();

    void parseMimeAppsList(const QString &file, int basePreference);

private:
    static QStringList mimeAppsDirs();

    void parseAddedAssociations(const KConfigGroup &group, const QString &file, int basePreference);
    void parseRemovedAssociations(const KConfigGroup &group, const QString &file);

    KOfferHash &m_offerHash;
    KServiceFactory *m_serviceFactory;
};

#endif /* KMIMEASSOCIATIONS_H */
