// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadfolderworker.h"
#include "common/vnoteforlder.h"
#include "db/vnotefolderoper.h"
#include "globaldef.h"

#include <DLog>

#include <QThread>

/**
 * @brief LoadFolderWorker::LoadFolderWorker
 * @param parent
 */
LoadFolderWorker::LoadFolderWorker(QObject *parent)
    : VNTask(parent)
{
}

/**
 * @brief LoadFolderWorker::run
 */
void LoadFolderWorker::run()
{
    static struct timeval start, backups, end;

    gettimeofday(&start, nullptr);
    backups = start;

    VNoteFolderOper folderOper;
    VNOTE_FOLDERS_MAP *foldersMap = folderOper.loadVNoteFolders();

    gettimeofday(&end, nullptr);

    qDebug() << "LoadFolderWorker(ms):" << TM(start, end);

    //TODO:
    //    Add load folder code here

    qDebug() << __FUNCTION__ << " load folders ok:" << foldersMap/* << " thread id:" << QThread::currentThreadId()*/;

    emit onFoldersLoaded(foldersMap);
}
