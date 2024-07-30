/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSYCOCAENTRY_H
#define KSYCOCAENTRY_H

#include <kservice_export.h>
#include <ksycocatype.h>

#include <QDataStream>
#include <QExplicitlySharedDataPointer>
#include <QStringList>
#include <QVariant>

#include <memory>

class KSycocaEntryPrivate;

/**
 * Base class for all Sycoca entries.
 *
 * You can't create an instance of KSycocaEntry, but it provides
 * the common functionality for servicetypes and services.
 *
 * @internal
 * @see http://techbase.kde.org/Development/Architecture/KDE3/System_Configuration_Cache
 */
class KSERVICE_EXPORT KSycocaEntry : public QSharedData
{
public:
    /*
     * constructs a invalid KSycocaEntry object
     */
    KSycocaEntry();

    virtual ~KSycocaEntry();

    /**
     * Returns true if this sycoca entry is of the given type.
     */
    bool isType(KSycocaType t) const;
    /**
     * internal
     */
    KSycocaType sycocaType() const;

    /**
     * A shared data pointer for KSycocaEntry.
     */
    typedef QExplicitlySharedDataPointer<KSycocaEntry> Ptr;
    /**
     * A list of shared data pointers for KSycocaEntry.
     */
    typedef QList<Ptr> List;

    /**
     * @return the name of this entry
     */
    QString name() const;

    /**
     * @return the path of this entry
     * The path can be absolute or relative.
     * The corresponding factory should know relative to what.
     */
    QString entryPath() const;

    /**
     * @return the unique ID for this entry
     * In practice, this is storageId() for KService and name() for everything else.
     * \since 4.2.1
     */
    QString storageId() const;

    /**
     * @return true if valid
     */
    bool isValid() const;

    /**
     * @return true if deleted
     */
    bool isDeleted() const;

    /**
     * Sets whether or not this service is deleted
     */
    void setDeleted(bool deleted);

    /**
     * @returns true, if this is a separator
     */
    bool isSeparator() const;

protected:
    KSERVICE_NO_EXPORT explicit KSycocaEntry(KSycocaEntryPrivate &d);
    std::unique_ptr<KSycocaEntryPrivate> const d_ptr;

private:
    // All these need access to offset()
    friend class KSycocaFactory;
    friend class KBuildServiceFactory;
    friend class KServiceFactory;
    friend class KService;
    friend class KSycocaDict;
    friend class KSycocaDictTest;

    /**
     * @internal
     * @return the position of the entry in the sycoca file
     */
    KSERVICE_NO_EXPORT int offset() const;

    Q_DISABLE_COPY(KSycocaEntry)

    Q_DECLARE_PRIVATE(KSycocaEntry)
};

#endif
