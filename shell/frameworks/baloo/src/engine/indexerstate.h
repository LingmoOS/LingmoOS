/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BALOO_INDEXER_STATE_H
#define BALOO_INDEXER_STATE_H

#include <QObject>
#include <QString>

#include "engine_export.h"

namespace Baloo {
Q_NAMESPACE_EXPORT(BALOO_ENGINE_EXPORT)

enum IndexerState {
        Idle,
        Suspended,
        FirstRun,
        NewFiles,
        ModifiedFiles,
        XAttrFiles,
        ContentIndexing,
        UnindexedFileCheck,
        StaleIndexEntriesClean,
        LowPowerIdle,
        Unavailable,
        Startup
};
Q_ENUM_NS(IndexerState)

BALOO_ENGINE_EXPORT QString stateString(IndexerState state);

//TODO: check for implicit conversion
BALOO_ENGINE_EXPORT QString stateString(int state);

}
#endif //BALOO_INDEXER_STATE_H
