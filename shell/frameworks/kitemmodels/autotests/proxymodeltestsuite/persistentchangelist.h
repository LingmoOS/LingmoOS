/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PERSISTENTCHANGELIST_H
#define PERSISTENTCHANGELIST_H

#include "indexfinder.h"

#include "proxymodeltestsuite_export.h"

struct PROXYMODELTESTSUITE_EXPORT PersistentIndexChange {
    IndexFinder parentFinder;
    int startRow;
    int endRow;
    int difference;
    bool toInvalid;
    QModelIndexList indexes;
    QList<QPersistentModelIndex> persistentIndexes;

    QModelIndexList descendantIndexes;
    QList<QPersistentModelIndex> persistentDescendantIndexes;
};

typedef QList<PersistentIndexChange> PersistentChangeList;

Q_DECLARE_METATYPE(PersistentChangeList)

#endif
