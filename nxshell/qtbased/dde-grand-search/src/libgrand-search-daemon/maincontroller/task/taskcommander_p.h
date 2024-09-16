// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKCOMMANDER_P_H
#define TASKCOMMANDER_P_H

#include "taskcommander.h"
#include "searcher/proxyworker.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QReadWriteLock>

namespace GrandSearch {

class TaskCommanderPrivate : public QObject
{
    Q_OBJECT
    friend class TaskCommander;
public:
    explicit TaskCommanderPrivate(TaskCommander *parent = nullptr);
    ~TaskCommanderPrivate();
    static void merge(MatchedItemMap &addTo, const MatchedItemMap &addFrom);
private:
    static void working(ProxyWorker *);
private slots:
    void onUnearthed(ProxyWorker *worker);
    void onFinished();
    void onWorkFinished(ProxyWorker *worker);
private:
    TaskCommander *q;
    volatile bool m_working = false;
    bool m_deleted = false;
    QString m_id;
    QString m_content;

    //当前所有的搜索结果和新数据缓冲区
    mutable QReadWriteLock m_lock;
    MatchedItemMap m_results;
    MatchedItemMap m_buffer;

    QList<ProxyWorker *> m_allWorkers;
    QList<ProxyWorker *> m_asyncWorkers;
    QList<ProxyWorker *> m_syncWorkers;
    QFutureWatcher<void> m_asyncLine;
    QFutureWatcher<void> m_syncLine;

    //未接收到停止信号的异步woker
    QList<ProxyWorker *> m_workingWorkers;
    bool m_finished = false;  //保证结束信号只发一次
};

}

#endif // TASKCOMMANDER_P_H
