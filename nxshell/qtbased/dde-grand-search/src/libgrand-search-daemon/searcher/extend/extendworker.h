// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDWORKER_H
#define EXTENDWORKER_H

#include "searcher/proxyworker.h"

namespace GrandSearch {

class PluginLiaison;
class ExtendWorker : public ProxyWorker
{
public:
    explicit ExtendWorker(const QString &name, QObject *parent = nullptr);
    ~ExtendWorker();
    bool setService(const QString &service, const QString &address,
                    const QString &interface, const QString &ver);
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
protected slots:
    void tryWorking();
    void onWorkFinished(const MatchedItemMap &ret);
private:
    enum CallState{NotCall, WaitCall, Called};
private:
    QAtomicInt m_status = Ready;
    QString m_context;
    QString m_taskID;

    PluginLiaison *m_liaison = nullptr;
    QMutex m_callMtx;
    volatile CallState m_callSerach = NotCall;
    QTimer m_timeout;

    //搜索结果
    mutable QMutex m_mtx;
    MatchedItemMap m_items;
};

}

#endif // EXTENDWORKER_H
