/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSYCOCADICT_H
#define KSYCOCADICT_H

#include "ksycocaentry.h"
#include <kservice_export.h>

#include <QList>

#include <memory>

class KSycocaDictPrivate;

class QString;
class QDataStream;

/**
 * @internal
 * Hash table implementation for the sycoca database file
 *
 * Only exported for the unit test
 */
class KSERVICE_EXPORT KSycocaDict // krazy:exclude=dpointer (not const because it gets deleted by clear())
{
public:
    /**
     * Create an empty dict, for building the database
     */
    KSycocaDict();
    /**
     * Create a dict from an existing database
     */
    KSycocaDict(QDataStream *str, int offset);

    ~KSycocaDict();

    /**
     * Adds a 'payload' to the dictionary with key 'key'.
     *
     * 'payload' should have a valid offset by the time
     * the dictionary gets saved.
     **/
    void add(const QString &key, const KSycocaEntry::Ptr &payload);

    /**
     * Removes the 'payload' from the dictionary with key 'key'.
     *
     * Not very fast, use with care O(N)
     **/
    void remove(const QString &key);

    /**
     * Looks up an entry identified by 'key'.
     *
     * If 0 is returned, no matching entry exists.
     * Otherwise, the offset of the entry is returned.
     *
     * NOTE: It is not guaranteed that this entry is
     * indeed the one you were looking for.
     * After loading the entry you should check that it
     * indeed matches the search key. If it doesn't
     * then no matching entry exists.
     */
    int find_string(const QString &key) const;

    /**
     * Looks up all entries identified by 'key'.
     * This is useful when the dict is used as a multi-hash.
     *
     * If an empty list is returned, no matching entry exists.
     * Otherwise, the offset of the matching entries are returned.
     *
     * NOTE: It is not guaranteed that each entry is
     * indeed among the ones you were looking for.
     * After loading each entry you should check that it
     * indeed matches the search key.
     */
    QList<int> findMultiString(const QString &key) const;

    /**
     * The number of entries in the dictionary.
     *
     * Only valid when building the database.
     */
    uint count() const;

    /**
     * Reset the dictionary.
     *
     * Only valid when building the database.
     */
    void clear();

    /**
     * Save the dictionary to the stream
     * A reasonable fast hash algorithm will be created.
     *
     * Typically this will find 90% of the entries directly.
     * Average hash table size: nrOfItems * 20 bytes.
     * Average duplicate list size: nrOfItms * avgKeyLength / 5.
     *
     * Unknown keys have an average 20% chance to give a false hit.
     * (That's why your program should check the result)
     *
     * Example:
     *   Assume 1000 items with an average key length of 60 bytes.
     *
     *   Approx. 900 items will hash directly to the right entry.
     *   Approx. 100 items require a lookup in the duplicate list.
     *
     *   The hash table size will be approx. 20Kb.
     *   The duplicate list size will be approx. 12Kb.
     **/
    void save(QDataStream &str);

private:
    Q_DISABLE_COPY(KSycocaDict)
    std::unique_ptr<KSycocaDictPrivate> d;
};

#endif
