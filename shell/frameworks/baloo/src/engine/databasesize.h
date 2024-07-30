/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DATABASE_SIZE_H
#define BALOO_DATABASE_SIZE_H

#include <lmdb.h>

namespace Baloo {

class DatabaseSize {
public:
    /**
     * This is the size which is computed with all the pages used from all the
     * individual database pages
     */
    size_t expectedSize;

    /**
     * This is the size based on the MDB_env and the total number of pages used
     */
    size_t actualSize;

    size_t postingDb;
    size_t positionDb;

    size_t docTerms;
    size_t docFilenameTerms;
    size_t docXattrTerms;

    size_t idTree;
    size_t idFilename;

    size_t docTime;
    size_t docData;

    size_t contentIndexingIds;
    size_t failedIds;

    size_t mtimeDb;
};

}
#endif
