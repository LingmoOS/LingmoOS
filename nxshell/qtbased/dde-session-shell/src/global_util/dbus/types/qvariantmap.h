// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef QVARIANTMAP_H
#define QVARIANTMAP_H

#include <QList>
#include <QVariant>
#include <QDBusMetaType>

typedef QMap<QString, QVariant> QVariantMap;

void registerQVariantMapMetaType();

#endif // QVARIANTMAP_H
