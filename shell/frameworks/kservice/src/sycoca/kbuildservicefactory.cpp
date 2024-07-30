/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2007 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbuildmimetypefactory_p.h"
#include "kbuildservicefactory_p.h"
#include "kbuildservicegroupfactory_p.h"
#include "ksycoca.h"

#include "ksycocadict_p.h"
#include "sycocadebug.h"
#include <KDesktopFile>

#include <QDebug>
#include <QDir>
#include <QMimeDatabase>

#include <QStandardPaths>
#include <kmimetypefactory_p.h>
#include <kservice_p.h>

KBuildServiceFactory::KBuildServiceFactory(KBuildMimeTypeFactory *mimeTypeFactory)
    : KServiceFactory(mimeTypeFactory->sycoca())
    , m_nameMemoryHash()
    , m_relNameMemoryHash()
    , m_menuIdMemoryHash()
    , m_dupeDict()
    , m_mimeTypeFactory(mimeTypeFactory)
{
    m_nameDict = new KSycocaDict();
    m_relNameDict = new KSycocaDict();
    m_menuIdDict = new KSycocaDict();
}

KBuildServiceFactory::~KBuildServiceFactory()
{
}

KService::Ptr KBuildServiceFactory::findServiceByDesktopName(const QString &name)
{
    return m_nameMemoryHash.value(name);
}

KService::Ptr KBuildServiceFactory::findServiceByDesktopPath(const QString &name)
{
    return m_relNameMemoryHash.value(name);
}

KService::Ptr KBuildServiceFactory::findServiceByMenuId(const QString &menuId)
{
    return m_menuIdMemoryHash.value(menuId);
}

KSycocaEntry *KBuildServiceFactory::createEntry(const QString &file) const
{
    const QStringView name = QStringView(file).mid(file.lastIndexOf(QLatin1Char('/')) + 1);

    // Is it a .desktop file?
    if (name.endsWith(QLatin1String(".desktop"))) {
        // qCDebug(SYCOCA) << file;

        Q_ASSERT(QDir::isAbsolutePath(file));
        KService *serv = new KService(file);

        // qCDebug(SYCOCA) << "Creating KService from" << file << "entryPath=" << serv->entryPath();
        // Note that the menuId will be set by the vfolder_menu.cpp code just after
        // createEntry returns.

        if (serv->isValid() && !serv->isDeleted()) {
            // qCDebug(SYCOCA) << "Creating KService from" << file << "entryPath=" << serv->entryPath() << "storageId=" << serv->storageId();
            return serv;
        } else {
            if (!serv->isDeleted()) {
                qCWarning(SYCOCA) << "Invalid Service : " << file;
            }
            delete serv;
            return nullptr;
        }
    } // TODO else if a Windows application,  new KService(name, exec, icon)
    return nullptr;
}

void KBuildServiceFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);

    str << qint32(m_nameDictOffset);
    str << qint32(m_relNameDictOffset);
    str << qint32(m_offerListOffset);
    str << qint32(m_menuIdDictOffset);
}

void KBuildServiceFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    m_nameDictOffset = str.device()->pos();
    m_nameDict->save(str);

    m_relNameDictOffset = str.device()->pos();
    m_relNameDict->save(str);

    saveOfferList(str);

    m_menuIdDictOffset = str.device()->pos();
    m_menuIdDict->save(str);

    qint64 endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

void KBuildServiceFactory::collectInheritedServices()
{
    // For each MIME type, go up the parent MIME type chains and collect offers.
    // For "removed associations" to work, we can't just grab everything from all parents.
    // We need to process parents before children, hence the recursive call in
    // collectInheritedServices(mime) and the QSet to process a given parent only once.
    QSet<QString> visitedMimes;
    const auto lst = m_mimeTypeFactory->allMimeTypes();
    for (const QString &mimeType : lst) {
        collectInheritedServices(mimeType, visitedMimes);
    }
}

void KBuildServiceFactory::collectInheritedServices(const QString &mimeTypeName, QSet<QString> &visitedMimes)
{
    if (visitedMimes.contains(mimeTypeName)) {
        return;
    }
    visitedMimes.insert(mimeTypeName);

    QMimeDatabase db;
    const QMimeType qmime = db.mimeTypeForName(mimeTypeName);
    const auto lst = qmime.parentMimeTypes();
    for (QString parentMimeType : lst) {
        // Workaround issue in shared-mime-info and/or Qt, which sometimes return an alias as parent
        parentMimeType = db.mimeTypeForName(parentMimeType).name();

        collectInheritedServices(parentMimeType, visitedMimes);

        const QList<KServiceOffer> &offers = m_offerHash.offersFor(parentMimeType);
        for (const auto &serviceOffer : offers) {
            if (!m_offerHash.hasRemovedOffer(mimeTypeName, serviceOffer.service())) {
                KServiceOffer offer(serviceOffer);
                offer.setMimeTypeInheritanceLevel(offer.mimeTypeInheritanceLevel() + 1);
                // qCDebug(SYCOCA) << "INHERITANCE: Adding service" << (*itserv).service()->entryPath() << "to" << mimeTypeName << "mimeTypeInheritanceLevel="
                // << mimeTypeInheritanceLevel;
                m_offerHash.addServiceOffer(mimeTypeName, offer);
            }
        }
    }
}

void KBuildServiceFactory::postProcessServices()
{
    // By doing all this here rather than in addEntry (and removing when replacing
    // with local override), we only do it for the final applications.
    // Note that this also affects resolution of the by-desktop-name lookup,
    // as name resolution is only performed *after* all the duplicates (based on
    // storage ID) have been removed.

    // For every service...
    for (auto itserv = m_entryDict->cbegin(), endIt = m_entryDict->cend(); itserv != endIt; ++itserv) {
        KSycocaEntry::Ptr entry = itserv.value();
        KService::Ptr service(static_cast<KService *>(entry.data()));

        const QString name = service->desktopEntryName();
        KService::Ptr dup = m_nameMemoryHash.value(name);
        if (dup) {
            // The rule is that searching for the desktop name "foo" should find
            // the desktop file with the storage id "foo.desktop" before it
            // finds "bar/foo.desktop" (or "bar-foo.desktop").
            // "bar/foo.desktop" and "baz/foo.desktop" are arbitrarily ordered
            // (in practice, the one later in the alphabet wins).
            if (dup->storageId().endsWith(service->storageId())) {
                // allow dup to be overridden
                m_nameDict->remove(name);
                dup = nullptr;
            }
        }
        if (!dup) {
            m_nameDict->add(name, entry);
            m_nameMemoryHash.insert(name, service);
        }

        const QString relName = service->entryPath();
        // qCDebug(SYCOCA) << "adding service" << service.data() << "isApp=" << service->isApplication() << "menuId=" << service->menuId() << "name=" << name <<
        // "relName=" << relName;
        m_relNameDict->add(relName, entry);
        m_relNameMemoryHash.insert(relName, service); // for KMimeAssociations

        const QString menuId = service->menuId();
        if (!menuId.isEmpty()) { // empty for services, non-empty for applications
            m_menuIdDict->add(menuId, entry);
            m_menuIdMemoryHash.insert(menuId, service); // for KMimeAssociations
        }
    }
    populateServiceTypes();
}

void KBuildServiceFactory::populateServiceTypes()
{
    QMimeDatabase db;
    // For every service...
    for (auto servIt = m_entryDict->cbegin(), endIt = m_entryDict->cend(); servIt != endIt; ++servIt) {
        KService::Ptr service(static_cast<KService *>(servIt.value().data()));
        const bool hidden = !service->showInCurrentDesktop();

        const auto mimeTypes = service->d_func()->m_mimeTypes;

        // Add this service to all its MIME types
        // Don't cache count(), it can change during iteration! (we can't use an iterator-based loop
        // here the container could get reallocated which would invalidate iterators)
        for (int i = 0; i < mimeTypes.count(); ++i) {
            const QString &mimeName = mimeTypes.at(i);

            if (hidden) {
                continue;
            }

            KServiceOffer offer(service, 1 /* preference; always 1 here, may be higher based on KMimeAssociations */, 0);
            QMimeType mime = db.mimeTypeForName(mimeName);
            if (!mime.isValid()) {
                if (mimeName.startsWith(QLatin1String("x-scheme-handler/"))) {
                    // Create those on demand
                    m_mimeTypeFactory->createFakeMimeType(mimeName);
                    m_offerHash.addServiceOffer(mimeName, offer);
                } else {
                    qCDebug(SYCOCA) << service->entryPath() << "specifies undefined MIME type/servicetype" << mimeName;
                    // technically we could call addServiceOffer here, 'mime' isn't used. But it
                    // would be useless, since we have no MIME type entry where to write the offers offset.
                    continue;
                }
            } else {
                bool shouldAdd = true;
                const auto lst = service->mimeTypes();

                for (const QString &otherType : lst) {
                    // Skip derived types if the base class is listed (#321706)
                    if (mimeName != otherType && mime.inherits(otherType)) {
                        // But don't skip aliases (they got resolved into mime.name() already, but don't let two aliases cancel out)
                        if (db.mimeTypeForName(otherType).name() != mime.name()) {
                            // qCDebug(SYCOCA) << "Skipping" << mime.name() << "because of" << otherType << "(canonical" << db.mimeTypeForName(otherType) <<
                            // ") while parsing" << service->entryPath();
                            shouldAdd = false;
                        }
                    }
                }
                if (shouldAdd) {
                    // qCDebug(SYCOCA) << "Adding service" << service->entryPath() << "to" << mime.name();
                    m_offerHash.addServiceOffer(mime.name(), offer); // mime.name() so that we resolve aliases
                }
            }
        }
    }

    // Read user preferences (added/removed associations) and add/remove serviceoffers to m_offerHash
    KMimeAssociations mimeAssociations(m_offerHash, this);
    mimeAssociations.parseAllMimeAppsList();

    // Now for each MIME type, collect services from parent MIME types
    collectInheritedServices();

    // Now collect the offsets into the (future) offer list
    // The loops look very much like the ones in saveOfferList obviously.
    int offersOffset = 0;
    const int offerEntrySize = sizeof(qint32) * 4; // four qint32s, see saveOfferList.

    const auto &offerHash = m_offerHash.serviceTypeData();
    auto it = offerHash.constBegin();
    const auto end = offerHash.constEnd();
    for (; it != end; ++it) {
        const QString stName = it.key();
        const ServiceTypeOffersData offersData = it.value();
        const int numOffers = offersData.offers.count();

        KMimeTypeFactory::MimeTypeEntry::Ptr entry = m_mimeTypeFactory->findMimeTypeEntryByName(stName);
        if (entry) {
            entry->setServiceOffersOffset(offersOffset);
            offersOffset += offerEntrySize * numOffers;
        } else if (stName.startsWith(QLatin1String("x-scheme-handler/"))) {
            // Create those on demand
            entry = m_mimeTypeFactory->createFakeMimeType(stName);
            entry->setServiceOffersOffset(offersOffset);
            offersOffset += offerEntrySize * numOffers;
        } else {
            if (stName.isEmpty()) {
                qCDebug(SYCOCA) << "Empty service type";
            } else {
                qCWarning(SYCOCA) << "Service type not found:" << stName;
            }
        }
    }
}

void KBuildServiceFactory::saveOfferList(QDataStream &str)
{
    m_offerListOffset = str.device()->pos();
    // qCDebug(SYCOCA) << "Saving offer list at offset" << m_offerListOffset;

    const auto &offerHash = m_offerHash.serviceTypeData();
    auto it = offerHash.constBegin();
    const auto end = offerHash.constEnd();
    for (; it != end; ++it) {
        const QString stName = it.key();
        const ServiceTypeOffersData offersData = it.value();
        QList<KServiceOffer> offers = offersData.offers;
        std::stable_sort(offers.begin(), offers.end()); // by initial preference

        int offset = -1;
        KMimeTypeFactory::MimeTypeEntry::Ptr entry = m_mimeTypeFactory->findMimeTypeEntryByName(stName);
        if (entry) {
            offset = entry->offset();
            // Q_ASSERT(str.device()->pos() == entry->serviceOffersOffset() + m_offerListOffset);
        }
        if (offset == -1) {
            qCDebug(SYCOCA) << "Didn't find servicetype or MIME type" << stName;
            continue;
        }

        for (const auto &offer : std::as_const(offers)) {
            // qCDebug(SYCOCA) << stName << ": writing offer" << offer.service()->desktopEntryName() << offset << offer.service()->offset() << "in sycoca at
            // pos" << str.device()->pos();
            Q_ASSERT(offer.service()->offset() != 0);

            str << qint32(offset);
            str << qint32(offer.service()->offset());
            str << qint32(offer.preference());
            str << qint32(offer.mimeTypeInheritanceLevel());
            // update offerEntrySize in populateServiceTypes if you add/remove something here
        }
    }

    str << qint32(0); // End of list marker (0)
}

void KBuildServiceFactory::addEntry(const KSycocaEntry::Ptr &newEntry)
{
    Q_ASSERT(newEntry);
    if (m_dupeDict.contains(newEntry)) {
        return;
    }

    const KService::Ptr service(static_cast<KService *>(newEntry.data()));
    m_dupeDict.insert(newEntry);
    KSycocaFactory::addEntry(newEntry);
}
