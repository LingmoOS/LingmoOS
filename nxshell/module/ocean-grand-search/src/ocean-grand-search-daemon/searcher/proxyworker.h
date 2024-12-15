// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROXYWORKER_H
#define PROXYWORKER_H

#include "global/matcheditem.h"

#include <QObject>

namespace GrandSearch {

//! 异步的Worker必须在析构函数中保证资源安全释放，且析构函数不能阻塞太久

class ProxyWorker : public QObject
{
    Q_OBJECT
public:
    enum Status {Ready,Runing,Completed,Terminated};
public:
    explicit ProxyWorker(const QString &name, QObject *parent = nullptr);
    QString name() const;
    virtual void setContext(const QString &context) = 0;
    virtual bool isAsync() const = 0;
    virtual bool working(void *context) = 0;
    virtual void terminate() = 0;
    virtual Status status() = 0;
    virtual bool hasItem() const = 0;
    virtual MatchedItemMap takeAll() = 0;
signals:
    void unearthed(ProxyWorker *worker);
    void asyncFinished(ProxyWorker *worker); //异步搜索结束信号
protected:
    QString m_name;
};

}

#endif // PROXYWORKER_H
