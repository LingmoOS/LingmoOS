// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DADSL_P_H
#define DADSL_P_H

#include "dadslsetting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAdslSettingPrivate
{
public:
    DAdslSettingPrivate();

    DNMSetting::SecretFlags m_passwordFlags;
    DAdslSetting::Protocol m_protocol;
    DAdslSetting::Encapsulation m_encapsulation;
    quint32 m_vpi;
    quint32 m_vci;
    QString m_name;
    QString m_username;
    QString m_password;
};

DNETWORKMANAGER_END_NAMESPACE
#endif