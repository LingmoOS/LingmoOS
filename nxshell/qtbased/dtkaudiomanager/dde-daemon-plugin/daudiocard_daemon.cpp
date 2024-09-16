// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiocard_daemon.h"
#include "daudioport.h"
#include "daudioport_daemon.h"
#include "daemonhelpers.hpp"

#include <QDBusArgument>
#include <QDebug>
#include <QDBusReply>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::DError;

DDaemonAudioCard::DDaemonAudioCard(QObject *parent)
    : DPlatformAudioCard (parent)
{
}

DDaemonAudioCard::~DDaemonAudioCard()
{

}

DDaemonAudioBluetoothCard::DDaemonAudioBluetoothCard(QObject *parent)
    : DDaemonAudioCard (parent)
    , m_inter(DDaemonInternal::newAudioInterface2(this))
{
    connect(this, &DDaemonAudioBluetoothCard::BluetoothAudioModeChanged, this, &DDaemonAudioBluetoothCard::modeChanged);
    connect(this, &DDaemonAudioBluetoothCard::BluetoothAudioModeOptsChanged, this, &DDaemonAudioBluetoothCard::modeOptionsChanged);
}

DDaemonAudioBluetoothCard::~DDaemonAudioBluetoothCard()
{

}

QString DDaemonAudioBluetoothCard::mode() const
{
    return qdbus_cast<QString>(m_inter->property("BluetoothAudioMode"));
}

QStringList DDaemonAudioBluetoothCard::modeOptions() const
{
    return qdbus_cast<QStringList>(m_inter->property("BluetoothAudioModeOpts"));
}

DExpected<void> DDaemonAudioBluetoothCard::setMode(const QString &mode)
{
    m_inter->call("SetBluetoothAudioMode", mode);
    QDBusReply<void> reply = m_inter->call("SetBluetoothAudioMode", mode);
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    }
    return {};
}
DAUDIOMANAGER_END_NAMESPACE
