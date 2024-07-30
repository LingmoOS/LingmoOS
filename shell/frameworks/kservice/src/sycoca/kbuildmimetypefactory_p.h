/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999-2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBUILD_MIME_TYPE_FACTORY_H
#define KBUILD_MIME_TYPE_FACTORY_H

#include <QStringList>
#include <kmimetypefactory_p.h>

/**
 * Mime-type factory for building ksycoca
 * @internal
 */
class KBuildMimeTypeFactory : public KMimeTypeFactory
{
public:
    /**
     * Create factory
     */
    explicit KBuildMimeTypeFactory(KSycoca *db);

    ~KBuildMimeTypeFactory() override;

    KSycocaEntry::List allEntries() const override;

    /**
     * Construct a KMimeType from a config file.
     */
    KSycocaEntry *createEntry(const QString &file) const override;

    MimeTypeEntry *createEntry(int) const override
    {
        assert(0);
        return nullptr;
    }

    KMimeTypeFactory::MimeTypeEntry::Ptr createFakeMimeType(const QString &name);

    /**
     * Write out MIME type specific index files.
     */
    void save(QDataStream &str) override;

    /**
     * Write out header information
     *
     * Don't forget to call the parent first when you override
     * this function.
     */
    void saveHeader(QDataStream &str) override;
};

#endif
