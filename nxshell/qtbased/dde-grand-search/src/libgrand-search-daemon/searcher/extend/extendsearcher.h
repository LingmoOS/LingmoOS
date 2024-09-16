// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDSEARCHER_H
#define EXTENDSEARCHER_H

#include "searcher/searcher.h"

namespace GrandSearch {

class ExtendSearcherPrivate;
class ExtendSearcher : public Searcher
{
    Q_OBJECT
    friend class ExtendSearcherPrivate;
public:
    enum Activatable{Inactivatable, InnerActivation, Trigger};
    explicit ExtendSearcher(const QString &name, QObject *parent = nullptr);
    void setService(const QString &service, const QString &address,
                    const QString &interface, const QString &ver);
    void setActivatable(Activatable);
    QString name() const;
    bool isActive() const;
    bool activate();
    ProxyWorker *createWorker() const;
    bool action(const QString &action, const QString &item);
signals:
    void activateRequest(const QString &name, bool &ret);
private:
    ExtendSearcherPrivate *d;
};

}

#endif // EXTENDSEARCHER_H
