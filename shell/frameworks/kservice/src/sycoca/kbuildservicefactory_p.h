/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999, 2007 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBUILD_SERVICE_FACTORY_H
#define KBUILD_SERVICE_FACTORY_H

#include <QStringList>

#include "kmimeassociations_p.h"
#include <kservicefactory_p.h>
// We export the services to the service group factory!
class KBuildServiceGroupFactory;
class KBuildMimeTypeFactory;

/**
 * Service factory for building ksycoca
 * @internal
 */
class KBuildServiceFactory : public KServiceFactory
{
public:
    /**
     * Create factory
     */
    KBuildServiceFactory(KBuildMimeTypeFactory *mimeTypeFactory);

    ~KBuildServiceFactory() override;

    /// Reimplemented from KServiceFactory
    KService::Ptr findServiceByDesktopName(const QString &name) override;
    /// Reimplemented from KServiceFactory
    KService::Ptr findServiceByDesktopPath(const QString &name) override;
    /// Reimplemented from KServiceFactory
    KService::Ptr findServiceByMenuId(const QString &menuId) override;

    /**
     * Construct a KService from a config file.
     */
    KSycocaEntry *createEntry(const QString &file) const override;

    KService *createEntry(int) const override
    {
        assert(0);
        return nullptr;
    }

    /**
     * Add a new entry.
     */
    void addEntry(const KSycocaEntry::Ptr &newEntry) override;

    /**
     * Write out service specific index files.
     */
    void save(QDataStream &str) override;

    /**
     * Write out header information
     *
     * Don't forget to call the parent first when you override
     * this function.
     */
    void saveHeader(QDataStream &str) override;

    void postProcessServices();

private:
    void populateServiceTypes();
    void saveOfferList(QDataStream &str);
    void collectInheritedServices();
    void collectInheritedServices(const QString &mime, QSet<QString> &visitedMimes);

    QHash<QString, KService::Ptr> m_nameMemoryHash; // m_nameDict is not usable while building ksycoca
    QHash<QString, KService::Ptr> m_relNameMemoryHash; // m_relNameDict is not usable while building ksycoca
    QHash<QString, KService::Ptr> m_menuIdMemoryHash; // m_menuIdDict is not usable while building ksycoca
    QSet<KSycocaEntry::Ptr> m_dupeDict;

    KOfferHash m_offerHash;

    KBuildMimeTypeFactory *m_mimeTypeFactory;
};

#endif
