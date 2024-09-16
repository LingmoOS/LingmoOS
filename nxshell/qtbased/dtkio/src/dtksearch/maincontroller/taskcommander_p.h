// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TASKCOMMANDER_P_H
#define TASKCOMMANDER_P_H

#include <QFutureWatcher>
#include <QReadWriteLock>

#include "dtksearch_global.h"

DSEARCH_BEGIN_NAMESPACE

class TaskCommander;
class AbstractSearcher;
class TaskCommanderPrivate : public QObject
{
    Q_OBJECT

public:
    explicit TaskCommanderPrivate(TaskCommander *parent);

    static void doSearch(AbstractSearcher *searcher);

public Q_SLOTS:
    void onUnearthed(AbstractSearcher *searcher);
    void onFinished();

public:
    TaskCommander *q = nullptr;
    QList<AbstractSearcher *> allSearchers;
    QFutureWatcher<void> futureWatcher;

    QStringList resultList;
    QReadWriteLock rwLock;

    volatile bool isWorking = false;
    bool deleted = false;
    bool finished = false;
};

DSEARCH_END_NAMESPACE

#endif // TASKCOMMANDER_P_H
