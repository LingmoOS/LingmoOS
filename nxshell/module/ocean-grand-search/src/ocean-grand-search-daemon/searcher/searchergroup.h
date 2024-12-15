// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHERGROUP_H
#define SEARCHERGROUP_H

#include "searcher.h"

#include <QObject>

namespace GrandSearch {

class SearcherGroupPrivate;
class SearcherGroup : public QObject
{
    Q_OBJECT
public:
    explicit SearcherGroup(QObject *parent = nullptr);

    bool init();
    QList<Searcher *> searchers() const;
    Searcher *searcher(const QString &name) const;
    void dormancy();
signals:

public slots:
private:
    SearcherGroupPrivate *d;
};

}

#endif // SEARCHERGROUP_H
