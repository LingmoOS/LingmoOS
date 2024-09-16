// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICSEARCHER_H
#define SEMANTICSEARCHER_H

#include "searcher/searcher.h"

namespace GrandSearch {
class SemanticSearcherPrivate;
class SemanticSearcher : public Searcher
{
    Q_OBJECT
    friend class SemanticSearcherPrivate;
public:
    explicit SemanticSearcher(QObject *parent = nullptr);
    ~SemanticSearcher();
    QString name() const Q_DECL_OVERRIDE;
    bool isActive() const Q_DECL_OVERRIDE;
    bool activate() Q_DECL_OVERRIDE;
    ProxyWorker *createWorker() const Q_DECL_OVERRIDE;
    bool action(const QString &action, const QString &item) Q_DECL_OVERRIDE;
private:
    SemanticSearcherPrivate *d;
};

}

#endif // SEMANTICSEARCHER_H
