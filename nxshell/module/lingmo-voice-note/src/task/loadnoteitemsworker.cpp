// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadnoteitemsworker.h"
#include "db/vnoteitemoper.h"
#include "globaldef.h"

#include <DLog>

#include <QThread>

/**
 * @brief LoadNoteItemsWorker::LoadNoteItemsWorker
 * @param parent
 */
LoadNoteItemsWorker::LoadNoteItemsWorker(QObject *parent)
    : VNTask(parent)
{
}

/**
 * @brief LoadNoteItemsWorker::run
 */
void LoadNoteItemsWorker::run()
{
    static struct timeval start, backups, end;

    gettimeofday(&start, nullptr);
    backups = start;

    VNoteItemOper notesOper;
    VNOTE_ALL_NOTES_MAP *notesMap = notesOper.loadAllVNotes();

    gettimeofday(&end, nullptr);

    qDebug() << "LoadNoteItemsWorker(ms):" << TM(start, end);

    //TODO:
    //    Add load folder code here

    qDebug() << __FUNCTION__ << " load all notes ok:" << notesMap->notes.size() /*<< " thread id:" << QThread::currentThreadId()*/;

    emit onAllNotesLoaded(notesMap);
}
