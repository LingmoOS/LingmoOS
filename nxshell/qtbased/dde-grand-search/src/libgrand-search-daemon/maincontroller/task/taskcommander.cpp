// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskcommander.h"
#include "taskcommander_p.h"

#include <QDateTime>
#include <QtConcurrent>

using namespace GrandSearch;

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent)
    , q(parent)
{

}

TaskCommanderPrivate::~TaskCommanderPrivate()
{

}

void TaskCommanderPrivate::merge(MatchedItemMap &addTo, const MatchedItemMap &addFrom)
{
    for (auto iter = addFrom.begin(); iter != addFrom.end(); ++iter) {
        MatchedItemMap::iterator org = addTo.find(iter.key());
        if (org == addTo.end()) {
            addTo.insert(iter.key(), iter.value());
        } else {
            org.value().append(iter.value());
        }
    }
}

void TaskCommanderPrivate::working(ProxyWorker *worker)
{
    Q_ASSERT(worker);
    worker->working(nullptr);
}

void TaskCommanderPrivate::onUnearthed(ProxyWorker *worker)
{
    Q_ASSERT(worker);
    auto isEmpty = [](const MatchedItemMap &map) {
        for (const MatchedItems &it : map.values()) {
            if (!it.isEmpty())
                return false;
        }
        return true;
    };

    if (m_allWorkers.contains(worker) && worker->hasItem()) {
        auto results = worker->takeAll();
        QWriteLocker lk(&m_lock);
        // check buffer has real item.
        bool emptyBuffer = isEmpty(m_buffer);

        merge(m_results, results);
        merge(m_buffer, results);

        bool emptyBuffer2 = isEmpty(m_buffer);
        //回到主线程发送信号
        if (emptyBuffer && !emptyBuffer2)
            QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection);
    }
}

void TaskCommanderPrivate::onFinished()
{
    qDebug() << __FUNCTION__ << m_workingWorkers.size() << m_finished << sender();
    //工作线程退出，若之前调用了deleteSelf那么在这里执行释放，否则发送结束信号
    if (m_asyncLine.isFinished() && m_syncLine.isFinished()) {
        if (m_deleted) {
            q->deleteLater();
            disconnect(q, nullptr, nullptr, nullptr);
        } else if (m_workingWorkers.isEmpty() && !m_finished) {
            m_finished = true;
            emit q->finished();
        }
    }
}

void TaskCommanderPrivate::onWorkFinished(ProxyWorker *worker)
{
    //检查
    ProxyWorker *send = dynamic_cast<ProxyWorker *>(sender());
    if (worker == nullptr || send != worker)
        return;

    m_workingWorkers.removeOne(worker);
    onFinished();
}

TaskCommander::TaskCommander(const QString &content, QObject *parent)
    : QObject(parent)
    , d(new TaskCommanderPrivate(this))
{
    d->m_id = QString::number(QDateTime::currentMSecsSinceEpoch());
    d->m_content = content;
}

TaskCommander::~TaskCommander()
{
    qDebug() << "task is deleted" << d->m_id;
}

QString TaskCommander::taskID() const
{
    return d->m_id;
}

QString TaskCommander::content() const
{
    return d->m_content;
}

void TaskCommander::setContent(const QString &content)
{
    d->m_content = content;
}

bool TaskCommander::start()
{
    qDebug() << "start" << this->taskID() << d->m_working
             << "async" << d->m_asyncWorkers.size() << "sync" << d->m_syncWorkers.size();
    if (d->m_working)
        return false;

    d->m_working = true;
    bool isOn = false;
    //所有异步搜索项在一个线程中依次执行
    if (!d->m_asyncWorkers.isEmpty()) {
        d->m_asyncLine.setFuture(QtConcurrent::run([this]() {
            QString taskID = d->m_id;
            for (auto worker : d->m_asyncWorkers) {
                if (!d->m_working)
                    return;

                if (worker->working(&taskID)) {
                    //在主线程处理搜索结束
                    d->connect(worker, &ProxyWorker::asyncFinished, d, &TaskCommanderPrivate::onWorkFinished ,Qt::QueuedConnection);
                    d->m_workingWorkers.append(worker);
                }
            }
        }));
        connect(&d->m_asyncLine, &QFutureWatcherBase::finished, d, &TaskCommanderPrivate::onFinished);
        isOn = true;
    }

    //所有同步搜索项在线程池中执行
    if (!d->m_syncWorkers.isEmpty()) {
        d->m_syncLine.setFuture(QtConcurrent::map(d->m_syncWorkers, TaskCommanderPrivate::working));
        connect(&d->m_syncLine, &QFutureWatcherBase::finished, d, &TaskCommanderPrivate::onFinished);
        isOn = true;
    }

    //无工作对象，直接结束。
    if (!isOn) {
        d->m_working = false;
        qWarning() << "no worker...";
        //加入队列，在start函数返回后发送结束信号
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

    return true;
}

void TaskCommander::stop()
{
    qDebug() << "stop" << this->taskID();
    d->m_asyncLine.cancel();
    d->m_syncLine.cancel();

    for (auto worker : d->m_allWorkers) {
        Q_ASSERT(worker);
        worker->terminate();
    }
    qDebug() << "worker stopped" << this->taskID();

    d->m_working = false;
    d->m_finished = true;
}

MatchedItemMap TaskCommander::getResults() const
{
    QReadLocker lk(&d->m_lock);
    return d->m_results;
}

MatchedItemMap TaskCommander::readBuffer() const
{
    QWriteLocker lk(&d->m_lock);
    decltype(d->m_buffer) ret;
    ret = std::move(d->m_buffer);
    return ret;
}

bool TaskCommander::isEmptyBuffer() const
{
    QReadLocker lk(&d->m_lock);
    return d->m_buffer.isEmpty();
}

bool TaskCommander::join(ProxyWorker *worker)
{
    //已经开启任务不允许添加搜索项
    if (d->m_working)
        return false;

    Q_ASSERT(worker);
    worker->setParent(d);
    worker->setContext(d->m_content);
    d->m_allWorkers.append(worker);

    if (worker->isAsync())
        d->m_asyncWorkers.append(worker);
    else
        d->m_syncWorkers.append(worker);

    //直连，在线程处理
    connect(worker, &ProxyWorker::unearthed, d, &TaskCommanderPrivate::onUnearthed, Qt::DirectConnection);
    return true;
}

void TaskCommander::deleteSelf()
{
    //工作线程未完全退出时不执行释放，待退出后再释放
    if (d->m_asyncLine.isFinished() && d->m_syncLine.isFinished())
        delete this;
    else
        d->m_deleted = true;
}

bool TaskCommander::isFinished() const
{
    return d->m_finished;
}

