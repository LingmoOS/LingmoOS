// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudioport_daemon.h"
#include "daudiocard_daemon.h"
#include "daemonhelpers.hpp"

#include <QDebug>
#include <QDBusAbstractInterface>
#include <QDBusArgument>
#include <QDBusReply>

DAUDIOMANAGER_BEGIN_NAMESPACE

DDaemonAudioPort::DDaemonAudioPort(DPlatformAudioCard *card)
    :  DPlatformAudioPort (card)
{
    Q_ASSERT(m_card);
}

DDaemonAudioPort::~DDaemonAudioPort()
{

}

void DDaemonAudioPort::setEnabled(const bool enabled)
{
    auto inter = DDaemonInternal::audioInterface();
    inter.call("SetPortEnabled", m_card->id(), m_name, enabled);
    DPlatformAudioPort::setEnabled(enabled);
}

bool DDaemonAudioPort::isEnabled() const
{
    auto inter = DDaemonInternal::audioInterface();
    QDBusReply<bool> reply = inter.call("IsPortEnabled", m_card->id(), m_name);
    return reply.value();
}

void DDaemonAudioPort::setActive(const int active)
{
    auto inter = DDaemonInternal::audioInterface();
    inter.call("SetPort", m_card->id(), m_name, m_direction);
    DPlatformAudioPort::setActive(active);
}
DAUDIOMANAGER_END_NAMESPACE
