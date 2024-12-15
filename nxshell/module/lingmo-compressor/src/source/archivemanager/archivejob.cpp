// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archivejob.h"

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
        // 设置结束标志
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
