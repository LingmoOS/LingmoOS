// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSOBJECTPATHLIST_H
#define DBUSOBJECTPATHLIST_H

#include <QDBusObjectPath>
#include <QDBusMetaType>
#include <QList>

typedef QList<QDBusObjectPath> DBusObjectPathList;

void registerDBusObjectPathListMetaType();

#endif // DBUSOBJECTPATHLIST_H
