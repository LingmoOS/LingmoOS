// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINLIAISON_P_H
#define PLUGINLIAISON_P_H

#include "pluginliaison.h"
#include "interface/searchplugininterfacev1.h"

namespace GrandSearch {

class PluginLiaisonPrivate : public QObject
{
    Q_OBJECT
    friend class PluginLiaison;
public:
    explicit PluginLiaisonPrivate(PluginLiaison *parent);
    ~PluginLiaisonPrivate();
    bool isVaild() const;
    static void parseResult(const QString &json, PluginLiaisonPrivate *d);
private slots:
    void onSearchReplied();
    void onServiceStarted(QString name, QString oldOwner, QString newOwner);
private:
    PluginLiaison *q;
    SearchPluginInterfaceV1 *m_inteface = nullptr;
    QString m_ver;
    QDBusPendingCallWatcher *m_replyWatcher = nullptr;
    QFuture<void> m_parseThread;
    QAtomicInteger<bool> m_searching = false;
    QString m_pluginName;
};

}

#endif // PLUGINLIAISON_P_H
