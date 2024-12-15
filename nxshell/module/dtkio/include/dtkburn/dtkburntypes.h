// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKBURNTYPES_H
#define DTKBURNTYPES_H

#include <QObject>

#include "dtkburn_global.h"

DBURN_BEGIN_NAMESPACE

enum class BurnOption : quint32 {
    KeepAppendable = 1,
    ISO9660Only = 1 << 1,
    JolietSupported = 1 << 2,
    RockRidgeSupported = 1 << 3,
    UDF102Supported = 1 << 4,

    JolietAndRockRidgeSupported = JolietSupported | RockRidgeSupported
};
Q_DECLARE_FLAGS(BurnOptions, BurnOption)

enum class MediaType : quint8 {
    NoMedia,
    CD_ROM,
    CD_R,
    CD_RW,
    DVD_ROM,
    DVD_R,
    DVD_RW,
    DVD_PLUS_R,
    DVD_PLUS_R_DL,
    DVD_RAM,
    DVD_PLUS_RW,
    BD_ROM,
    BD_R,
    BD_RE
};

enum class JobStatus : quint8 {
    Failed,
    Idle,
    Running,
    Stalled,
    Finished
};

DBURN_END_NAMESPACE

#endif   // DTKBURNTYPES_H
