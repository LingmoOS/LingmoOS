// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "archivejob.h"

DCOMPRESSOR_USE_NAMESPACE
ArchiveJob::ArchiveJob(QObject *parent)
    : QObject(parent)
{
}

ArchiveJob::~ArchiveJob()
{
}

void ArchiveJob::kill()
{
    if (doKill()) {
        m_eFinishedType = PFT_Cancel;
        m_eErrorType = ET_UserCancelOpertion;
        finishJob();
    }
}

bool ArchiveJob::status()
{
    return true;
}

void ArchiveJob::finishJob()
{
    emit signalJobFinshed();
    deleteLater();
}

bool ArchiveJob::doKill()
{
    return false;
}
