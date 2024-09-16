// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ITERATORSEARCHER_H
#define ITERATORSEARCHER_H

#include <QElapsedTimer>
#include <QMutex>
#include <QRegularExpression>

#include "dtksearch_global.h"
#include "searcher/abstractsearcher.h"

DSEARCH_BEGIN_NAMESPACE

class IteratorSearcher : public AbstractSearcher
{
    Q_OBJECT
public:
    explicit IteratorSearcher(const QVariantMap &options, QObject *parent = nullptr);
    ~IteratorSearcher() override;

    void search() override;
    void stop() override;
    bool hasItem() const override;
    QStringList takeAll() override;

    void tryNotify();
    void doSearch();

private:
    QAtomicInt status = Ready;
    QStringList allResults;
    mutable QMutex mutex;
    QStringList searchPathList;
    QRegularExpression regex;
    quint32 resultCount = 0;

    QElapsedTimer notifyTimer;
    int lastEmit = 0;
};

DSEARCH_END_NAMESPACE

#endif // ITERATORSEARCHER_H
