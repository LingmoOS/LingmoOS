// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLIAISON_H
#define PLUGINLIAISON_H

#include "global/matcheditem.h"

#include <QObject>

namespace GrandSearch {

class PluginLiaisonPrivate;
class PluginLiaison : public QObject
{
    Q_OBJECT
public:
    explicit PluginLiaison(QObject *parent = nullptr);
    bool init(const QString &service, const QString &address,
              const QString &interface, const QString &ver,
              const QString &pluginName);
    bool isVaild() const;
    bool search(const QString &task, const QString &context);
    bool stop(const QString &task);
    bool action(const QString &type, const QString &item);
signals:
    void searchFinished(const MatchedItemMap &ret);
    void ready();
private:
    PluginLiaisonPrivate *d;
};

}

#endif // PLUGINLIAISON_H
