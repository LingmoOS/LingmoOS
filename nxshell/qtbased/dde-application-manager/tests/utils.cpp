// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "global.h"

bool registerObjectToDBus(QObject *, const QString &, const QString &)
{
    return true;
}

void unregisterObjectFromDBus(const QString &)
{
    return;
}
