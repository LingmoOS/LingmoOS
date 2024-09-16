// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OLDDATALOADWOKERS_H
#define OLDDATALOADWOKERS_H

#include "common/datatypedef.h"
#include "task/vntask.h"

class OldDataLoadTask : public VNTask
{
    Q_OBJECT
public:
    explicit OldDataLoadTask(QObject *parent = nullptr);

protected:
    //加载老数据库数据
    virtual void run();
signals:
    //数据加载完成
    void finishLoad();
};

class OldDataUpgradeTask : public VNTask
{
    Q_OBJECT
public:
    explicit OldDataUpgradeTask(QObject *parent = nullptr);

protected:
    //升级数据
    virtual void run();
signals:
    //升级进度
    void progressValue(qint32);
    //升级完成
    void finishUpgrade();
};

#endif // OLDDATALOADWOKERS_H
