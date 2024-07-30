/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef CACHE_H
#define CACHE_H

#include <QObject>
#include <QSet>

#include "entry.h"
#include "provider.h"

#include "knewstuffcore_export.h"

#include <memory.h>

namespace KNSCore
{
class CachePrivate;
class KNEWSTUFFCORE_EXPORT Cache : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns an instance of a shared cache for appName
     * That way it is made sure, that there do not exist different
     * instances of cache, with different contents
     * @param appName The file name of the registry - this is usually
     * the application name, it will be stored in "apps/knewstuff3/appname.knsregistry"
     */
    static QSharedPointer<Cache> getCache(const QString &appName);

    ~Cache() override;

    /// Read the installed entries (on startup)
    void readRegistry();
    /// All entries that have been installed by a certain provider
    Entry::List registryForProvider(const QString &providerId);

    /// All entries known by the cache (this corresponds with entries which are installed, regardless of status)
    Entry::List registry() const;

    /// Save the list of installed entries
    void writeRegistry();

    void insertRequest(const KNSCore::Provider::SearchRequest &, const KNSCore::Entry::List &entries);
    Entry::List requestFromCache(const KNSCore::Provider::SearchRequest &);

    /**
     * This will run through all entries in the cache, and remove all entries
     * where all the installed files they refer to no longer exist.
     *
     * This cannot be done wholesale for all caches, as some consumers will allow
     * this to happen (or indeed expect it to), and so we have to do this on a
     * per-type basis
     *
     * This will also cause the cache store to be updated
     *
     * @since 5.71
     */
    void removeDeletedEntries();

    /**
     * Get the entry which installed the passed file. If no entry lists the
     * passed file as having been installed by it, an invalid entry will be
     * returned.
     * @param installedFile The full path name for an installed file
     * @return An entry if one was found, or an invalid entry if no entry says it installed that file
     * since 5.74
     */
    KNSCore::Entry entryFromInstalledFile(const QString &installedFile) const;

    /**
     * Emitted when the cache has changed underneath us, and need users of the cache to know
     * that this has happened.
     * @param entry The entry which has changed
     * @since 5.75
     */
    Q_SIGNAL void entryChanged(const KNSCore::Entry &entry);

public Q_SLOTS:
    void registerChangedEntry(const KNSCore::Entry &entry);

private:
    Q_DISABLE_COPY(Cache)
    Cache(const QString &appName);

private:
    std::unique_ptr<CachePrivate> d;
};

}

#endif
