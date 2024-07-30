/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999-2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kservice.h"
#include "kservicefactory_p.h"
#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocatype.h"
#include "servicesdebug.h"
#include <QDir>
#include <QFile>

extern int servicesDebugArea();

KServiceFactory::KServiceFactory(KSycoca *db)
    : KSycocaFactory(KST_KServiceFactory, db)
    , m_nameDict(nullptr)
    , m_relNameDict(nullptr)
    , m_menuIdDict(nullptr)
{
    m_offerListOffset = 0;
    m_nameDictOffset = 0;
    m_relNameDictOffset = 0;
    m_menuIdDictOffset = 0;
    if (!sycoca()->isBuilding()) {
        QDataStream *str = stream();
        if (!str) {
            qWarning() << "Could not open sycoca database, you must run kbuildsycoca first!";
            return;
        }
        // Read Header
        qint32 i;
        (*str) >> i;
        m_nameDictOffset = i;
        (*str) >> i;
        m_relNameDictOffset = i;
        (*str) >> i;
        m_offerListOffset = i;
        (*str) >> i;
        m_menuIdDictOffset = i;

        const qint64 saveOffset = str->device()->pos();
        // Init index tables
        m_nameDict = new KSycocaDict(str, m_nameDictOffset);
        // Init index tables
        m_relNameDict = new KSycocaDict(str, m_relNameDictOffset);
        // Init index tables
        m_menuIdDict = new KSycocaDict(str, m_menuIdDictOffset);
        str->device()->seek(saveOffset);
    }
}

KServiceFactory::~KServiceFactory()
{
    delete m_nameDict;
    delete m_relNameDict;
    delete m_menuIdDict;
}

KService::Ptr KServiceFactory::findServiceByName(const QString &_name)
{
    if (!sycocaDict()) {
        return KService::Ptr(); // Error!
    }

    // Warning : this assumes we're NOT building a database
    // But since findServiceByName isn't called in that case...
    // [ see KServiceTypeFactory for how to do it if needed ]

    int offset = sycocaDict()->find_string(_name);
    if (!offset) {
        return KService::Ptr(); // Not found
    }

    KService::Ptr newService(createEntry(offset));

    // Check whether the dictionary was right.
    if (newService && (newService->name() != _name)) {
        // No it wasn't...
        return KService::Ptr();
    }
    return newService;
}

KService::Ptr KServiceFactory::findServiceByDesktopName(const QString &_name)
{
    if (!m_nameDict) {
        return KService::Ptr(); // Error!
    }

    // Warning : this assumes we're NOT building a database
    // KBuildServiceFactory reimplements it for the case where we are building one

    int offset = m_nameDict->find_string(_name);
    if (!offset) {
        return KService::Ptr(); // Not found
    }

    KService::Ptr newService(createEntry(offset));

    // Check whether the dictionary was right.
    if (newService && (newService->desktopEntryName() != _name)) {
        // No it wasn't...
        return KService::Ptr();
    }
    return newService;
}

KService::Ptr KServiceFactory::findServiceByDesktopPath(const QString &_name)
{
    if (!m_relNameDict) {
        return KService::Ptr(); // Error!
    }

    // Warning : this assumes we're NOT building a database
    // KBuildServiceFactory reimplements it for the case where we are building one

    int offset = m_relNameDict->find_string(_name);
    if (!offset) {
        // qCDebug(SERVICES) << "findServiceByDesktopPath:" << _name << "not found";
        return KService::Ptr(); // Not found
    }

    KService::Ptr newService(createEntry(offset));
    if (!newService) {
        qCDebug(SERVICES) << "createEntry failed!";
    }
    // Check whether the dictionary was right
    // It's ok that it's wrong, for the case where we're looking up an unknown service,
    // and the hash value gave us another one.
    if (newService && (newService->entryPath() != _name)) {
        // No it wasn't...
        return KService::Ptr();
    }
    return newService;
}

KService::Ptr KServiceFactory::findServiceByMenuId(const QString &_menuId)
{
    if (!m_menuIdDict) {
        return KService::Ptr(); // Error!
    }

    // Warning : this assumes we're NOT building a database
    // KBuildServiceFactory reimplements it for the case where we are building one

    int offset = m_menuIdDict->find_string(_menuId);
    if (!offset) {
        return KService::Ptr(); // Not found
    }

    KService::Ptr newService(createEntry(offset));

    // Check whether the dictionary was right.
    if (newService && (newService->menuId() != _menuId)) {
        // No it wasn't...
        return KService::Ptr();
    }
    return newService;
}

KService::Ptr KServiceFactory::findServiceByStorageId(const QString &_storageId)
{
    KService::Ptr service = findServiceByMenuId(_storageId);
    if (service) {
        return service;
    }

    service = findServiceByDesktopPath(_storageId);
    if (service) {
        return service;
    }

    if (!QDir::isRelativePath(_storageId) && QFile::exists(_storageId)) {
        return KService::Ptr(new KService(_storageId));
    }

    QString tmp = _storageId;
    tmp = tmp.mid(tmp.lastIndexOf(QLatin1Char('/')) + 1); // Strip dir

    if (tmp.endsWith(QLatin1String(".desktop"))) {
        tmp.chop(8);
    }

    if (tmp.endsWith(QLatin1String(".kdelnk"))) {
        tmp.chop(7);
    }

    service = findServiceByDesktopName(tmp);

    return service;
}

KService *KServiceFactory::createEntry(int offset) const
{
    KSycocaType type;
    QDataStream *str = sycoca()->findEntry(offset, type);
    if (type != KST_KService) {
        qCWarning(SERVICES) << "KServiceFactory: unexpected object entry in KSycoca database (type=" << int(type) << ")";
        return nullptr;
    }
    KService *newEntry = new KService(*str, offset);
    if (!newEntry->isValid()) {
        qCWarning(SERVICES) << "KServiceFactory: corrupt object in KSycoca database!";
        delete newEntry;
        newEntry = nullptr;
    }
    return newEntry;
}

KService::List KServiceFactory::allServices()
{
    KService::List result;
    const KSycocaEntry::List list = allEntries();
    for (const auto &entryPtr : list) {
        if (entryPtr->isType(KST_KService)) {
            result.append(KService::Ptr(static_cast<KService *>(entryPtr.data())));
        }
    }
    return result;
}

QStringList KServiceFactory::resourceDirs()
{
    return KSycocaFactory::allDirectories(QStringLiteral("applications"));
}

QList<KServiceOffer> KServiceFactory::offers(int serviceTypeOffset, int serviceOffersOffset)
{
    QList<KServiceOffer> list;

    // Jump to the offer list
    QDataStream *str = stream();
    str->device()->seek(m_offerListOffset + serviceOffersOffset);

    qint32 aServiceTypeOffset;
    qint32 aServiceOffset;
    qint32 offerPreference;
    qint32 mimeTypeInheritanceLevel;
    while (true) {
        (*str) >> aServiceTypeOffset;
        if (aServiceTypeOffset) {
            (*str) >> aServiceOffset;
            (*str) >> offerPreference;
            (*str) >> mimeTypeInheritanceLevel;
            if (aServiceTypeOffset == serviceTypeOffset) {
                // Save stream position !
                const qint64 savedPos = str->device()->pos();
                // Create Service
                KService *serv = createEntry(aServiceOffset);
                if (serv) {
                    KService::Ptr servPtr(serv);
                    list.append(KServiceOffer(servPtr, 1, mimeTypeInheritanceLevel));
                }
                // Restore position
                str->device()->seek(savedPos);
            } else {
                break; // too far
            }
        } else {
            break; // 0 => end of list
        }
    }
    return list;
}

KService::List KServiceFactory::serviceOffers(int serviceTypeOffset, int serviceOffersOffset)
{
    KService::List list;

    // Jump to the offer list
    QDataStream *str = stream();
    str->device()->seek(m_offerListOffset + serviceOffersOffset);

    qint32 aServiceTypeOffset;
    qint32 aServiceOffset;
    qint32 offerPreference;
    qint32 mimeTypeInheritanceLevel;
    while (true) {
        (*str) >> aServiceTypeOffset;
        if (aServiceTypeOffset) {
            (*str) >> aServiceOffset;
            (*str) >> offerPreference; // unused (remove once KMimeTypeTrader/KServiceTypeTrader are gone)
            (*str) >> mimeTypeInheritanceLevel; // unused (remove once KMimeTypeTrader/KServiceTypeTrader are gone)
            if (aServiceTypeOffset == serviceTypeOffset) {
                // Save stream position !
                const qint64 savedPos = str->device()->pos();
                // Create service
                KService *serv = createEntry(aServiceOffset);
                if (serv) {
                    list.append(KService::Ptr(serv));
                }
                // Restore position
                str->device()->seek(savedPos);
            } else {
                break; // too far
            }
        } else {
            break; // 0 => end of list
        }
    }
    return list;
}

bool KServiceFactory::hasOffer(int serviceTypeOffset, int serviceOffersOffset, int testedServiceOffset)
{
    // Save stream position
    QDataStream *str = stream();
    const qint64 savedPos = str->device()->pos();

    // Jump to the offer list
    str->device()->seek(m_offerListOffset + serviceOffersOffset);
    bool found = false;
    qint32 aServiceTypeOffset;
    qint32 aServiceOffset;
    qint32 offerPreference;
    qint32 mimeTypeInheritanceLevel;
    while (!found) {
        (*str) >> aServiceTypeOffset;
        if (aServiceTypeOffset) {
            (*str) >> aServiceOffset;
            (*str) >> offerPreference;
            (*str) >> mimeTypeInheritanceLevel;
            if (aServiceTypeOffset == serviceTypeOffset) {
                if (aServiceOffset == testedServiceOffset) {
                    found = true;
                }
            } else {
                break; // too far
            }
        } else {
            break; // 0 => end of list
        }
    }
    // Restore position
    str->device()->seek(savedPos);
    return found;
}

void KServiceFactory::virtual_hook(int id, void *data)
{
    KSycocaFactory::virtual_hook(id, data);
}
