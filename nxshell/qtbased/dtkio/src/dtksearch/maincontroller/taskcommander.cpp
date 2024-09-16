// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QtConcurrent/QtConcurrentMap>

#include "taskcommander.h"
#include "taskcommander_p.h"
#include "dtksearchtypes.h"
#include "searcher/fulltext/fulltextsearcher.h"
#include "searcher/iterator/iteratorsearcher.h"

DSEARCH_BEGIN_NAMESPACE

TaskCommanderPrivate::TaskCommanderPrivate(TaskCommander *parent)
    : QObject(parent),
      q(parent)
{
}

void TaskCommanderPrivate::doSearch(AbstractSearcher *searcher)
{
    searcher->search();
}

void TaskCommanderPrivate::onUnearthed(AbstractSearcher *searcher)
{
    Q_ASSERT(searcher);

    if (allSearchers.contains(searcher) && searcher->hasItem()) {
        auto results = searcher->takeAll();
        QWriteLocker lk(&rwLock);
        bool isEmpty = resultList.isEmpty();

        resultList += results;
        if (isEmpty)
            QMetaObject::invokeMethod(q, "matched", Qt::QueuedConnection);
    }
}

void TaskCommanderPrivate::onFinished()
{
    qDebug() << __FUNCTION__ << allSearchers.size() << finished << sender();
    if (futureWatcher.isFinished()) {
        if (deleted) {
            q->deleteLater();
            disconnect(q, nullptr, nullptr, nullptr);
        } else if (!finished) {
            finished = true;
            emit q->finished();
        }
    }
}

TaskCommander::TaskCommander(const QVariantMap &options, QObject *parent)
    : QObject(parent),
      d(new TaskCommanderPrivate(this))
{
    createSearcher(options);
}

QStringList TaskCommander::getResults() const
{
    QReadLocker lk(&d->rwLock);
    return std::move(d->resultList);
}

bool TaskCommander::start()
{
    if (d->isWorking)
        return false;

    d->isWorking = true;
    bool isOn = false;

    if (!d->allSearchers.isEmpty()) {
        d->futureWatcher.setFuture(QtConcurrent::map(d->allSearchers, TaskCommanderPrivate::doSearch));
        connect(&d->futureWatcher, &QFutureWatcherBase::finished, d, &TaskCommanderPrivate::onFinished);
        isOn = true;
    }

    if (!isOn) {
        d->isWorking = false;
        qWarning() << __FUNCTION__ << "no searcher...";
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
    }

    return true;
}

void TaskCommander::stop()
{
    d->futureWatcher.cancel();

    for (auto searcher : d->allSearchers) {
        Q_ASSERT(searcher);
        searcher->stop();
    }

    d->isWorking = false;
    d->finished = true;
}

void TaskCommander::deleteSelf()
{
    if (d->futureWatcher.isFinished())
        delete this;
    else
        d->deleted = true;
}

void TaskCommander::createSearcher(const QVariantMap &options)
{
    SearchFlags flags = options.value("SearchFlags").value<SearchFlags>();
    if (flags.testFlag(SearchFlag::FileName)) {
        // TODO(liuzhangjian): file name search
        auto searcher = new IteratorSearcher(options, this);
        d->allSearchers << searcher;
    }

    if (flags.testFlag(SearchFlag::FullText)) {
        auto searcher = new FullTextSearcher(options, this);
        d->allSearchers << searcher;
    }

    for (const auto searcher : d->allSearchers)
        connect(searcher, &AbstractSearcher::unearthed, d, &TaskCommanderPrivate::onUnearthed);
}

DSEARCH_END_NAMESPACE
