// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DVPNSETTING_P_H
#define DVPNSETTING_P_H

#include "dvpnsetting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DVpnSettingPrivate
{
public:
    DVpnSettingPrivate();

    bool m_persistent;
    uint m_timeout;
    QString m_name;
    QString m_serviceType;
    QString m_username;
    QMap<QString, QString> m_data;
    QMap<QString, QString> m_secrets;
};

DNETWORKMANAGER_END_NAMESPACE

#endif