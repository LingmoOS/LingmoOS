// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNTASK_H
#define VNTASK_H

#include <QObject>
#include <QRunnable>
//任务基类，线程任务继承此类，可放入线程池
class VNTask : public QObject
    , public QRunnable
{
    Q_OBJECT
public:
    explicit VNTask(QObject *parent = nullptr);

signals:

public slots:
};

#endif // VNTASK_H
