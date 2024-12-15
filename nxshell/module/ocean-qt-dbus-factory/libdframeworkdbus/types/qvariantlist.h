// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QVARIANTLIST_H
#define QVARIANTLIST_H

#include <QList>
#include <QVariant>
#include <QDBusMetaType>

typedef QList<QVariant> QVariantList;

void registerQVariantListMetaType();

#endif // QVARIANTLIST_H
