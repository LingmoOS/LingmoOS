// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNMSETTING_P_H
#define DNMSETTING_P_H

#include "dnmsetting.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

#if !NM_CHECK_VERSION(1, 16, 0)
#define NM_SETTING_WIREGUARD_SETTING_NAME "wireguard"
#endif

class DNMSettingPrivate
{
public:
    DNMSettingPrivate();
    ~DNMSettingPrivate() = default;

    DNMSetting::SettingType m_type;
    bool m_init;
};

DNETWORKMANAGER_END_NAMESPACE

#endif