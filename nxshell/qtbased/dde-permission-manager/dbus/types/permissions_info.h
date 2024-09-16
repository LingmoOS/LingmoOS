// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PERMISSIONSINFO
#define PERMISSIONSINFO

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, QString> PermissionsInfo;
Q_DECLARE_METATYPE(PermissionsInfo)

void registerPermissionsInfoMetaType();

#endif // PERMISSIONSINFO
