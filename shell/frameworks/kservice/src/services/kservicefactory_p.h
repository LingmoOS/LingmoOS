/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1998, 1999 Torben Weis <weis@kde.org>
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICEFACTORY_P_H
#define KSERVICEFACTORY_P_H

#include <QStringList>

#include "kserviceoffer.h"
#include "ksycocafactory_p.h"
#include <assert.h>

class KSycoca;
class KSycocaDict;

/**
 * @internal
 * A sycoca factory for services (e.g. applications)
 * It loads the services from parsing directories (e.g. prefix/share/applications/)
 * but can also create service from data streams or single config files
 *
 * Exported for unit tests
 */
class KSERVICE_EXPORT KServiceFactory : public KSycocaFactory
{
    K_SYCOCAFACTORY(KST_KServiceFactory)
public:
    /**
     * Create factory
     */
    explicit KServiceFactory(KSycoca *sycoca);
    ~KServiceFactory() override;

    /**
     * Construct a KService from a config file.
     */
    KSycocaEntry *createEntry(const QString &) const override
    {
        assert(0);
        return nullptr;
    }

    /**
     * Find a service (by translated name, e.g. "Terminal")
     * (Not virtual because not used inside kbuildsycoca4, only an external service for some KDE apps)
     */
    KService::Ptr findServiceByName(const QString &_name);

    /**
     * Find a service (by desktop file name, e.g. "konsole")
     */
    virtual KService::Ptr findServiceByDesktopName(const QString &_name);

    /**
     * Find a service ( by desktop path, e.g. "System/konsole.desktop")
     */
    virtual KService::Ptr findServiceByDesktopPath(const QString &_name);

    /**
     * Find a service ( by menu id, e.g. "kde-konsole.desktop")
     */
    virtual KService::Ptr findServiceByMenuId(const QString &_menuId);

    KService::Ptr findServiceByStorageId(const QString &_storageId);

    /**
     * @return the services supporting the given service type
     * The @p serviceOffersOffset allows to jump to the right entries directly.
     */
    KServiceOfferList offers(int serviceTypeOffset, int serviceOffersOffset);

    /**
     * @return the services supporting the given service type
     * The @p serviceOffersOffset allows to jump to the right entries directly.
     */
    KService::List serviceOffers(int serviceTypeOffset, int serviceOffersOffset);

    /**
     * Test if a specific service is associated with a specific servicetype
     * @param serviceTypeOffset the offset of the service type being tested
     * @param serviceOffersOffset allows to jump to the right entries for the service type directly.
     * @param testedServiceOffset the offset of the service being tested
     */
    bool hasOffer(int serviceTypeOffset, int serviceOffersOffset, int testedServiceOffset);

    /**
     * @return all services. Very memory consuming, avoid using.
     */
    KService::List allServices();

    /**
     * Returns the directories to watch for this factory.
     */
    static QStringList resourceDirs();

    /**
     * @return the unique service factory, creating it if necessary
     */
    static KServiceFactory *self();

protected:
    KService *createEntry(int offset) const override;

    // All those variables are used by KBuildServiceFactory too
    int m_offerListOffset;
    KSycocaDict *m_nameDict;
    int m_nameDictOffset;
    KSycocaDict *m_relNameDict;
    int m_relNameDictOffset;
    KSycocaDict *m_menuIdDict;
    int m_menuIdDictOffset;

protected:
    void virtual_hook(int id, void *data) override;

private:
    class KServiceFactoryPrivate *d;
};

#endif
