// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWLIST_H
#define WINDOWLIST_H

#include <QObject>

typedef QList<quint32> WindowList;
Q_DECLARE_METATYPE(WindowList)

void registerWindowListMetaType();

#endif // WINDOWLIST_H
