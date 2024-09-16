// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWIRELESSSECURITY_P_H
#define DWIRELESSSECURITY_P_H

#include "dwirelesssecuritysetting.h"
#include <QList>
#include <QString>

DNETWORKMANAGER_BEGIN_NAMESPACE

class DWirelessSecuritySettingPrivate
{
public:
    DWirelessSecuritySettingPrivate();

    quint32 m_wepTxKeyidx;
    DWirelessSecuritySetting::KeyMgmt m_keyMgmt;
    DWirelessSecuritySetting::AuthAlg m_authAlg;
    DNMSetting::SecretFlags m_wepKeyFlags;
    DWirelessSecuritySetting::WepKeyType m_wepKeyType;
    DNMSetting::SecretFlags m_pskFlags;
    DNMSetting::SecretFlags m_leapPasswordFlags;
    DWirelessSecuritySetting::Pmf m_pmf;
    QString m_leapUsername;
    QString m_wepKey0;
    QString m_wepKey1;
    QString m_wepKey2;
    QString m_wepKey3;
    QString m_name;
    QString m_psk;
    QString m_leapPassword;
    QList<DWirelessSecuritySetting::WpaProtocolVersion> m_proto;
    QList<DWirelessSecuritySetting::WpaEncryptionCapabilities> m_pairwise;
    QList<DWirelessSecuritySetting::WpaEncryptionCapabilities> m_group;
};

DNETWORKMANAGER_END_NAMESPACE

#endif
