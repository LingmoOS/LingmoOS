// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#ifndef BATTERYPRESENTINFO_H
#define BATTERYPRESENTINFO_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, bool> BatteryPresentInfo;

void registerBatteryPresentInfoMetaType();

#endif // BATTERYPRESENTINFO_H
