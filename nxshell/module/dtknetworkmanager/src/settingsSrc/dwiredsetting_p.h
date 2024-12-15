// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRED_P_H
#define DWIRED_P_H

#include "dwiredsetting.h"
#include <QMap>
#include <QStringList>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWiredSettingPrivate
{
public:
    DWiredSettingPrivate();

    bool m_autoNegotiate;
    quint32 m_speed;
    quint32 m_mtu;
    DWiredSetting::PortType m_port;
    DWiredSetting::DuplexType m_duplex;
    DWiredSetting::S390Nettype m_s390NetType;
    DWiredSetting::WakeOnLanFlags m_wakeOnLan;
    QByteArray m_macAddress;
    QString m_name;
    QString m_generateMacAddressMask;
    QString m_wakeOnLanPassword;
    QString m_assignedMacAddress;
    QStringList m_macAddressBlacklist;
    QStringList m_s390Subchannels;
    QMap<QString, QString> m_s390Options;
};

DNETWORKMANAGER_END_NAMESPACE

#endif
