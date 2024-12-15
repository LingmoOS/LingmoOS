// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PERMISSIONSINFO
#define PERMISSIONSINFO

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, QString> PermissionsInfo;
Q_DECLARE_METATYPE(PermissionsInfo)

void registerPermissionsInfoMetaType();

#endif // PERMISSIONSINFO
