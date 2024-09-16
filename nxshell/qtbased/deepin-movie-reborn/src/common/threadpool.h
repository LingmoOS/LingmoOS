// Copyright (C) 2017 ~ 2018 Wuhan Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QThread>

#include <singleton.h>

class ThreadPool : public QObject, public DMovie::DSingleton<ThreadPool>
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = 0);
    ~ThreadPool();

    QThread *newThread();
    void moveToNewThread(QObject *obj);
    void quitAll();

private:
    friend class DMovie::DSingleton<ThreadPool>;

    QList<QThread *> m_pool;
};

