// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef POLICYDETAILS_H
#define POLICYDETAILS_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, QString> PolicyDetails;

void registerPolicyDetailsMetaType();

#endif // POLICYDETAILS_H
