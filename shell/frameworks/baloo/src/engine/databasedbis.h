/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_DATABASE_DBIS_H
#define BALOO_DATABASE_DBIS_H

#include <lmdb.h>

namespace Baloo {

class DatabaseDbis {
public:
    MDB_dbi postingDbi = 0;
    MDB_dbi positionDBi = 0;

    MDB_dbi docTermsDbi = 0;
    MDB_dbi docFilenameTermsDbi = 0;
    MDB_dbi docXattrTermsDbi = 0;

    MDB_dbi idTreeDbi = 0;
    MDB_dbi idFilenameDbi = 0;

    MDB_dbi docTimeDbi = 0;
    MDB_dbi docDataDbi = 0;
    MDB_dbi contentIndexingDbi = 0;

    MDB_dbi mtimeDbi = 0;
    MDB_dbi failedIdDbi = 0;

    DatabaseDbis() = default;

    bool isValid() {
        return postingDbi && positionDBi && docTermsDbi && docFilenameTermsDbi && docXattrTermsDbi &&
               idTreeDbi && idFilenameDbi && docTimeDbi && docDataDbi && contentIndexingDbi && mtimeDbi
               && failedIdDbi;
    }
};

}
#endif
