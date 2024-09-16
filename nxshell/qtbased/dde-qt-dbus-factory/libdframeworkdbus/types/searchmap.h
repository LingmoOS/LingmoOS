// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCH_H
#define SEARCH_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, double> SearchMap;

void registerSearchMapMetaType();

#endif // SEARCH_H
