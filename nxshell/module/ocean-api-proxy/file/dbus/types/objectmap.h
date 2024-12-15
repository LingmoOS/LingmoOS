// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#ifndef OBJECTMAP_H
#define OBJECTMAP_H

#include <QMap>
#include <QVariant>
#include <QDBusMetaType>

using ObjectInterfaceMap = QMap<QString, QVariantMap>;
using ObjectMap = QMap<QDBusObjectPath, ObjectInterfaceMap>;

void registerObjectMapMetaType();

#endif // OBJECTMAP_H

