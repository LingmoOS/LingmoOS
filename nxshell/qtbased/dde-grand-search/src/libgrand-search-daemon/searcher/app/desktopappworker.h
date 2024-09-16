// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOPAPPWORKER_H
#define DESKTOPAPPWORKER_H

#include "searcher/proxyworker.h"

#include <QMutex>

namespace GrandSearch {

class DesktopAppWorker : public ProxyWorker
{
    Q_OBJECT
public:
    explicit DesktopAppWorker(const QString &name, QObject *parent = nullptr);
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
public:
    void setIndexTable(const QHash<QString, QList<QSharedPointer<MatchedItem>>> &table);
private:
    QString group() const;
    QString buildKeyword(const QString &keyword);
private:
    QAtomicInt m_status = Ready;
    QString m_context;
    QHash<QString, QList<QSharedPointer<MatchedItem>>> m_indexTable;

    //搜索结果
    mutable QMutex m_mtx;
    MatchedItems m_items;
};

}

#endif // DESKTOPAPPWORKER_H
