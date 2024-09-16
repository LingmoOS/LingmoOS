// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>

#include "anythingsearcher.h"
#include "utils.h"

DSEARCH_BEGIN_NAMESPACE

AnythingSearcher::AnythingSearcher(const QVariantMap &options, QObject *parent)
    : AbstractSearcher(options, parent)
{
}

AnythingSearcher::~AnythingSearcher() {}

void AnythingSearcher::search()
{
    if (!status.testAndSetRelease(Ready, Runing))
        return;
    // TODO: liuzhangjian
}

void AnythingSearcher::stop()
{
    status.storeRelease(Terminated);
}

bool AnythingSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QStringList AnythingSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

void AnythingSearcher::tryNotify()
{
    int cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > Utils::kEmitInterval) {
        lastEmit = cur;
        qDebug() << "IteratorSearcher unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

DSEARCH_END_NAMESPACE
