// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTUNSETTING_P_H
#define DTUNSETTING_P_H

#include "dtunsetting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DTunSettingPrivate
{
public:
    DTunSettingPrivate();

    bool m_pi;
    bool m_vnetHdr;
    bool m_multiQueue;
    DTunSetting::Mode m_mode;
    QString m_name;
    QString m_group;
    QString m_owner;
};

DNETWORKMANAGER_END_NAMESPACE

#endif