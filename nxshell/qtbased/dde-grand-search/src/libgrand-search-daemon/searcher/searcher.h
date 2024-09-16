// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHER_H
#define SEARCHER_H

#include <QObject>

namespace GrandSearch {

class ProxyWorker;
class Searcher : public QObject
{
    Q_OBJECT
public:
    explicit Searcher(QObject *parent = nullptr);
    virtual QString name() const = 0;
    virtual bool isActive() const = 0;
    virtual bool activate();
    virtual ProxyWorker *createWorker() const = 0;
    virtual bool action(const QString &action, const QString &item) = 0;
};

}

#endif // SEARCHER_H
