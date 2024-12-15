// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ANYTHINGSEARCHER_H
#define ANYTHINGSEARCHER_H

#include <QMutex>
#include <QElapsedTimer>

#include "dtksearch_global.h"
#include "searcher/abstractsearcher.h"

DSEARCH_BEGIN_NAMESPACE

class AnythingSearcher : public AbstractSearcher
{
    Q_OBJECT
public:
    explicit AnythingSearcher(const QVariantMap &options, QObject *parent = nullptr);
    ~AnythingSearcher() override;

    void search() override;
    void stop() override;
    bool hasItem() const override;
    QStringList takeAll() override;

    void tryNotify();
    static bool canSearch(const QString &path);

private:
    QAtomicInt status = Ready;
    QStringList allResults;
    mutable QMutex mutex;

    QElapsedTimer notifyTimer;
    int lastEmit = 0;
};

DSEARCH_END_NAMESPACE

#endif  // ANYTHINGSEARCHER_H
