// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KEYBOARDLAYOUTLIST_H
#define KEYBOARDLAYOUTLIST_H

#include <QMap>
#include <QString>
#include <QObject>
#include <QDBusMetaType>

typedef QMap<QString, QString> KeyboardLayoutList;

void registerKeyboardLayoutListMetaType();

#endif // KEYBOARDLAYOUTLIST_H
