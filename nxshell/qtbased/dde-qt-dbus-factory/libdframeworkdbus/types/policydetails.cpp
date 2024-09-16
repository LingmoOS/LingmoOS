// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policydetails.h"

void registerPolicyDetailsMetaType()
{
    qRegisterMetaType<PolicyDetails>("PolicyDetails");
    qDBusRegisterMetaType<PolicyDetails>();
}
