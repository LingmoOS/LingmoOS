// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2022 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */
#include "queryrulesfirewalldjob.h"
#include <KLocalizedString>
#include <QTimer>

QueryRulesFirewalldJob::QueryRulesFirewalldJob()
{
    m_simple = new FirewalldJob("getServices", {""}, FirewalldJob::SIMPLELIST);
    m_direct = new FirewalldJob("getAllRules");

    connect(m_direct, &KJob::result, this, [this](void) {
        m_directFinished = true;

        if (m_direct->error()) {
            setError(m_direct->error());
            setErrorText(m_direct->errorString());
            emitResult();
            return;
        }

        m_replyDirect = m_direct->getFirewalldreply();
        if (m_simpleFinished) {
            emitResult();
        }
    });

    connect(m_simple, &KJob::result, this, [this](void) {
        m_simpleFinished = true;

        if (m_simple->error()) {
            setError(m_simple->error());
            setErrorText(m_simple->errorString());
            emitResult();
            return;
        }

        m_replyServices = m_simple->getServices();
        if (m_directFinished) {
            emitResult();
        }
    });
}

QString QueryRulesFirewalldJob::name() const
{
    return i18n("firewalld listing rules and services");
}

QList<firewalld_reply> QueryRulesFirewalldJob::getFirewalldreply() const
{
    return m_replyDirect;
}

QStringList QueryRulesFirewalldJob::getServices() const
{
    return m_replyServices;
}

void QueryRulesFirewalldJob::start()
{
    m_direct->start();
    m_simple->start();
    return;
}

QueryRulesFirewalldJob::~QueryRulesFirewalldJob() = default;
