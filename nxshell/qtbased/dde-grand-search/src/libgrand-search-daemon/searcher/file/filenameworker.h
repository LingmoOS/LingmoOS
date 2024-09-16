// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMEWORKER_H
#define FILENAMEWORKER_H

#include "searcher/proxyworker.h"

class ComDeepinAnythingInterface;

namespace GrandSearch {

class FileNameWorkerPrivate;
class FileNameWorker : public ProxyWorker
{
    Q_OBJECT
public:
    explicit FileNameWorker(const QString &name, bool supportParallelSearch, QObject *parent = nullptr);
    ~FileNameWorker();
    void setContext(const QString &context) Q_DECL_OVERRIDE;
    bool isAsync() const Q_DECL_OVERRIDE;
    bool working(void *context) Q_DECL_OVERRIDE;
    void terminate() Q_DECL_OVERRIDE;
    Status status() Q_DECL_OVERRIDE;
    bool hasItem() const Q_DECL_OVERRIDE;
    MatchedItemMap takeAll() Q_DECL_OVERRIDE;
private:
    FileNameWorkerPrivate *d_ptr;
    Q_DECLARE_PRIVATE(FileNameWorker)
};

}

#endif // FILENAMEWORKER_H
