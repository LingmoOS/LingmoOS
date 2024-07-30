/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2006-2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMIMETYPEFACTORY_H
#define KMIMETYPEFACTORY_H

#include <assert.h>

#include <QStringList>

#include "ksycocafactory_p.h"

class KSycoca;

/**
 * @internal  - this header is not installed
 *
 * A sycoca factory for MIME type entries
 * This is only used to point to the "service offers" in ksycoca for each MIME type.
 * @see KMimeType
 */
class KMimeTypeFactory : public KSycocaFactory
{
    K_SYCOCAFACTORY(KST_KMimeTypeFactory)
public:
    /**
     * Create factory
     */
    explicit KMimeTypeFactory(KSycoca *db);

    ~KMimeTypeFactory() override;

    /**
     * Not meant to be called at this level
     */
    KSycocaEntry *createEntry(const QString &) const override
    {
        assert(0);
        return nullptr;
    }

    /**
     * Returns the possible offset for a given MIME type entry.
     */
    int entryOffset(const QString &mimeTypeName);

    /**
     * Returns the offset into the service offers for a given MIME type.
     */
    int serviceOffersOffset(const QString &mimeTypeName);

    /**
     * Returns the directories to watch for this factory.
     */
    static QStringList resourceDirs();

public:
    /**
     * @return all MIME types
     * Slow and memory consuming, avoid using
     */
    QStringList allMimeTypes();

    /**
     * @return the unique MIME type factory, creating it if necessary
     */
    static KMimeTypeFactory *self();

public: // public for KBuildServiceFactory
    // A small entry for each MIME type with name and offset into the services-offer-list.
    class MimeTypeEntryPrivate;
    class KSERVICE_EXPORT MimeTypeEntry : public KSycocaEntry
    {
        Q_DECLARE_PRIVATE(MimeTypeEntry)
    public:
        typedef QExplicitlySharedDataPointer<MimeTypeEntry> Ptr;

        MimeTypeEntry(const QString &file, const QString &name);
        MimeTypeEntry(QDataStream &s, int offset);
        ~MimeTypeEntry() override;

        int serviceOffersOffset() const;
        void setServiceOffersOffset(int off);
    };

    MimeTypeEntry::Ptr findMimeTypeEntryByName(const QString &name);

protected:
    MimeTypeEntry *createEntry(int offset) const override;

private:
    // d pointer: useless since this header is not installed
    // class KMimeTypeFactoryPrivate* d;
};

#endif
