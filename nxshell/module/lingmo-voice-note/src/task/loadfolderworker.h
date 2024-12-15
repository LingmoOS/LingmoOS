// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADFOLDERWORKER_H
#define LOADFOLDERWORKER_H

#include "common/datatypedef.h"
#include "vntask.h"

#include <QObject>
#include <QRunnable>

//加载记事本数据线程
class LoadFolderWorker : public VNTask
{
    Q_OBJECT
public:
    explicit LoadFolderWorker(QObject *parent = nullptr);

protected:
    //加载数据
    virtual void run();
signals:
    //完成加载
    void onFoldersLoaded(VNOTE_FOLDERS_MAP *foldesMap);
public slots:
};

#endif // LOADFOLDERWORKER_H
