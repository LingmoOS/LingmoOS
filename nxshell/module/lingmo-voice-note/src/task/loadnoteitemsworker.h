// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADNOTEITEMSWORKER_H
#define LOADNOTEITEMSWORKER_H

#include "common/datatypedef.h"
#include "vntask.h"

#include <QObject>
#include <QRunnable>
#include <QtGlobal>
//加载记事项线程
class LoadNoteItemsWorker : public VNTask
{
    Q_OBJECT
public:
    explicit LoadNoteItemsWorker(QObject *parent = nullptr);

protected:
    //加载数据
    virtual void run();
signals:
    //加载完成
    void onAllNotesLoaded(VNOTE_ALL_NOTES_MAP *foldesMap);
public slots:

protected:
};

#endif // LOADNOTEITEMSWORKER_H
