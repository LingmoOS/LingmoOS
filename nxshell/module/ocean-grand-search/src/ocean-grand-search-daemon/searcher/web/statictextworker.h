// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STATICTEXTWORKER_H
#define STATICTEXTWORKER_H

#include "searcher/proxyworker.h"

namespace GrandSearch {

class StaticTextWorker : public ProxyWorker
{
    Q_OBJECT
public:
    explicit StaticTextWorker(const QString &name, QObject *parent = nullptr);
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
private:
    QString group() const;
    QString createUrl(const QString &searchEngine) const;
private:
    QAtomicInt m_status = Ready;
    QString m_context;

    //搜索结果
    mutable QMutex m_mtx;
    MatchedItems m_items;
};

}

#endif // STATICTEXTWORKER_H
