// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DRECENTMANAGER_H
#define DRECENTMANAGER_H

#include "dtkcore_global.h"
#include <QString>

DCORE_BEGIN_NAMESPACE

struct LIBDTKCORESHARED_EXPORT DRecentData
{
    QString appName;
    QString appExec;
    QString mimeType;
};

class LIBDTKCORESHARED_EXPORT DRecentManager
{
public:
    static bool addItem(const QString &uri, DRecentData &data);
    static void removeItem(const QString &target);
    static void removeItems(const QStringList &list);
};

DCORE_END_NAMESPACE

#endif // DRECENTMANAGER_H
