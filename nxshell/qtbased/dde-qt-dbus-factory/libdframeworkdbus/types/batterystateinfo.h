// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BATTERYSTATEINFO_H
#define BATTERYSTATEINFO_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, quint32> BatteryStateInfo;

void registerBatteryStateInfoMetaType();

#endif // BATTERYSTATEINFO_H
