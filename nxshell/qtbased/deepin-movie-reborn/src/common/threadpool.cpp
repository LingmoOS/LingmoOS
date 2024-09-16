// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "threadpool.h"

#include <QDebug>

ThreadPool::ThreadPool(QObject *parent) : QObject(parent)
{
}

ThreadPool::~ThreadPool()
{
    quitAll();
}

QThread *ThreadPool::newThread()
{
    auto thread = new QThread;
//    qInfo() << "add <<<<<<<" << thread;
    m_pool.push_back(thread);
    return thread;
}

void ThreadPool::moveToNewThread(QObject *obj)
{
    auto work = newThread();
    obj->moveToThread(work);
    work->start();
}

void ThreadPool::quitAll()
{
    for (auto thread : m_pool) {
//        qInfo() << thread;
        thread->quit();
        thread->wait(2000);
    }
    qInfo() << "all thread quit";
}
