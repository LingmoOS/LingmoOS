// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2022 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */
#ifndef QUERYRULESFIREWALLDJOB_H
#define QUERYRULESFIREWALLDJOB_H

#include "dbustypes.h"
#include "firewalldjob.h"
#include <KJob>
#include <QList>

class QueryRulesFirewalldJob : public KJob
{
    Q_OBJECT

public:
    QueryRulesFirewalldJob();
    ~QueryRulesFirewalldJob();

    void start() override;
    QList<firewalld_reply> getFirewalldreply() const;
    QStringList getServices() const;
    QString name() const;
    // Q_SIGNAL void queryFinish();

private:
    FirewalldJob *m_direct;
    FirewalldJob *m_simple;
    bool m_directFinished = false;
    bool m_simpleFinished = false;
    QList<firewalld_reply> m_replyDirect = {};
    QStringList m_replyServices = {};
};

#endif
