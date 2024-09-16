// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BATTERYPERCENTAGEINFO_H
#define BATTERYPERCENTAGEINFO_H

#include <QMap>
#include <QDBusMetaType>

typedef QMap<QString, double> BatteryPercentageInfo;

void registerBatteryPercentageInfoMetaType();

#endif // BATTERYPERCENTAGEINFO_H
