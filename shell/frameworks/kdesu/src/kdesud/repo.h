/* vi: ts=8 sts=4 sw=4

    This file is part of the KDE project, module kdesu.
    SPDX-FileCopyrightText: 1999, 2000 Geert Jansen <jansen@kde.org>
*/

#ifndef __Repo_h_included__
#define __Repo_h_included__

#include <QByteArray>
#include <QMap>

/**
 * Used internally.
 */
struct Data_entry {
    QByteArray value;
    QByteArray group;
    unsigned int timeout;
};

/**
 * String repository.
 *
 * This class implements a string repository with expiration.
 */
class Repository
{
public:
    Repository();
    ~Repository();

    /** Remove data elements which are expired. */
    int expire();

    /** Add a data element */
    void add(const QByteArray &key, Data_entry &data);

    /** Delete a data element. */
    int remove(const QByteArray &key);

    /** Delete all data entries having the given group.  */
    int removeGroup(const QByteArray &group);

    /** Delete all data entries based on key. */
    int removeSpecialKey(const QByteArray &key);

    /** Checks for the existence of the specified group. */
    int hasGroup(const QByteArray &group) const;

    /** Return a data value.  */
    QByteArray find(const QByteArray &key) const;

    /** Returns the key values for the given group. */
    QByteArray findKeys(const QByteArray &group, const char *sep = "-") const;

private:
    QMap<QByteArray, Data_entry> repo;
    typedef QMap<QByteArray, Data_entry>::Iterator RepoIterator;
    typedef QMap<QByteArray, Data_entry>::ConstIterator RepoCIterator;
    unsigned head_time;
};

#endif
