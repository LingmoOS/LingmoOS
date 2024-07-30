/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2006-2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmimetypefactory_p.h"
#include "ksycocaentry_p.h"
#include "servicesdebug.h"
#include <QDataStream>
#include <ksycoca.h>
#include <ksycocadict_p.h>

extern int servicesDebugArea();

KMimeTypeFactory::KMimeTypeFactory(KSycoca *db)
    : KSycocaFactory(KST_KMimeTypeFactory, db)
{
}

KMimeTypeFactory::~KMimeTypeFactory()
{
}

int KMimeTypeFactory::entryOffset(const QString &mimeTypeName)
{
    if (!sycocaDict()) {
        return -1; // Error!
    }
    assert(!sycoca()->isBuilding());
    const int offset = sycocaDict()->find_string(mimeTypeName.toLower());
    return offset;
}

int KMimeTypeFactory::serviceOffersOffset(const QString &mimeTypeName)
{
    const int offset = entryOffset(mimeTypeName.toLower());
    if (!offset) {
        return -1; // Not found
    }

    MimeTypeEntry::Ptr newMimeType(createEntry(offset));
    if (!newMimeType) {
        return -1;
    }
    // Check whether the dictionary was right.
    if (newMimeType->name() != mimeTypeName.toLower()) {
        // No it wasn't...
        return -1;
    }
    return newMimeType->serviceOffersOffset();
}

KMimeTypeFactory::MimeTypeEntry *KMimeTypeFactory::createEntry(int offset) const
{
    KSycocaType type;
    QDataStream *str = sycoca()->findEntry(offset, type);
    if (!str) {
        return nullptr;
    }

    if (type != KST_KMimeTypeEntry) {
        qCWarning(SERVICES) << "KMimeTypeFactory: unexpected object entry in KSycoca database (type=" << int(type) << ")";
        return nullptr;
    }
    MimeTypeEntry *newEntry = new MimeTypeEntry(*str, offset);
    if (newEntry && !newEntry->isValid()) {
        qCWarning(SERVICES) << "KMimeTypeFactory: corrupt object in KSycoca database!\n";
        delete newEntry;
        newEntry = nullptr;
    }
    return newEntry;
}

QStringList KMimeTypeFactory::allMimeTypes()
{
    // TODO: reimplement in terms of "listing xdgdata-mime", to avoid ksycoca dependency,
    // then move to KMimeTypeRepository
    const KSycocaEntry::List list = allEntries();
    QStringList result;
    result.reserve(list.size());

    std::transform(list.cbegin(), list.cend(), std::back_inserter(result), [](const KSycocaEntry::Ptr &entry) {
        Q_ASSERT(entry->isType(KST_KMimeTypeEntry));

        MimeTypeEntry::Ptr mimeType(static_cast<MimeTypeEntry *>(entry.data()));
        return mimeType->name();
    });

    return result;
}

KMimeTypeFactory::MimeTypeEntry::Ptr KMimeTypeFactory::findMimeTypeEntryByName(const QString &name)
{
    Q_ASSERT(sycoca()->isBuilding());
    // We're building a database - the MIME type entry must be in memory
    KSycocaEntry::Ptr servType = m_entryDict->value(name.toLower());
    return MimeTypeEntry::Ptr(static_cast<MimeTypeEntry *>(servType.data()));
}

QStringList KMimeTypeFactory::resourceDirs()
{
    return KSycocaFactory::allDirectories(QStringLiteral("mime"));
}

////

class KMimeTypeFactory::MimeTypeEntryPrivate : public KSycocaEntryPrivate
{
public:
    K_SYCOCATYPE(KST_KMimeTypeEntry, KSycocaEntryPrivate)
    MimeTypeEntryPrivate(const QString &file, const QString &name)
        : KSycocaEntryPrivate(file)
        , m_name(name)
        , m_serviceOffersOffset(-1)
    {
    }
    MimeTypeEntryPrivate(QDataStream &s, int offset)
        : KSycocaEntryPrivate(s, offset)
        , m_serviceOffersOffset(-1)
    {
        s >> m_name >> m_serviceOffersOffset;
    }
    QString name() const override
    {
        return m_name;
    }
    void save(QDataStream &s) override;

    QString m_name;
    int m_serviceOffersOffset;
};

void KMimeTypeFactory::MimeTypeEntryPrivate::save(QDataStream &s)
{
    KSycocaEntryPrivate::save(s);
    s << m_name << m_serviceOffersOffset;
}

////

KMimeTypeFactory::MimeTypeEntry::MimeTypeEntry(const QString &file, const QString &name)
    : KSycocaEntry(*new MimeTypeEntryPrivate(file, name.toLower()))
{
}

KMimeTypeFactory::MimeTypeEntry::MimeTypeEntry(QDataStream &s, int offset)
    : KSycocaEntry(*new MimeTypeEntryPrivate(s, offset))
{
}

KMimeTypeFactory::MimeTypeEntry::~MimeTypeEntry()
{
}

int KMimeTypeFactory::MimeTypeEntry::serviceOffersOffset() const
{
    Q_D(const MimeTypeEntry);
    return d->m_serviceOffersOffset;
}

void KMimeTypeFactory::MimeTypeEntry::setServiceOffersOffset(int off)
{
    Q_D(MimeTypeEntry);
    d->m_serviceOffersOffset = off;
}
