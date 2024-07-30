/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2015 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "indexerstate.h"

#include <KLocalizedString>

QString Baloo::stateString(IndexerState state)
{
    QString status = i18n("Unknown");
    switch (state) {
    case Idle:
        status = i18n("Idle");
        break;
    case Suspended:
        status =  i18n("Suspended");
        break;
    case FirstRun:
        status =  i18n("Initial Indexing");
        break;
    case NewFiles:
        status = i18n("Indexing new files");
        break;
    case ModifiedFiles:
        status = i18n("Indexing modified files");
        break;
    case XAttrFiles:
        status = i18n("Indexing Extended Attributes");
        break;
    case ContentIndexing:
        status = i18n("Indexing file content");
        break;
    case UnindexedFileCheck:
        status = i18n("Checking for unindexed files");
        break;
    case StaleIndexEntriesClean:
        status = i18n("Checking for stale index entries");
        break;
    case LowPowerIdle:
        status = i18n("Idle (Powersave)");
        break;
    case Unavailable:
        status = i18n("Not Running");
        break;
    case Startup:
        status = i18n("Starting");
        break;
    }
    return status;
}

QString Baloo::stateString(int state)
{
    return stateString(static_cast<IndexerState>(state));
}

#include "moc_indexerstate.cpp"
