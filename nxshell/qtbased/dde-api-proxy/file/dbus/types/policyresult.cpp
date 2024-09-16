// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#include <QDBusMetaType>

#include "policyresult.h"

void registerPolicyResultMetaType()
{
    qRegisterMetaType<PolicyResult>("PolicyResult");
    qDBusRegisterMetaType<PolicyResult>();
}
