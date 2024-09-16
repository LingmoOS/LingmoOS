// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILENAMESEARCHER_H
#define FILENAMESEARCHER_H

#include "searcher/searcher.h"

namespace GrandSearch {

class FileNameSearcher : public Searcher
{
    Q_OBJECT
public:
    explicit FileNameSearcher(QObject *parent = nullptr);
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
    bool supportParallelSearch() const;
};

}

#endif // FILENAMESEARCHER_H
