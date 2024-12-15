// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICWORKER_H
#define SEMANTICWORKER_H

#include "searcher/proxyworker.h"

namespace GrandSearch {
class SemanticWorkerPrivate;
class SemanticWorker : public ProxyWorker
{
    Q_OBJECT
    friend class SemanticWorkerPrivate;
public:
    explicit SemanticWorker(const QString &name, QObject *parent = nullptr);
    ~SemanticWorker();
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
    void setEngineState(bool e, bool v, bool f);
private:
    SemanticWorkerPrivate *d;
};

}

#endif // SEMANTICWORKER_H
