// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
// 
// SPDX-License-Identifier: GPL-3.0-or-later


#include "batterypercentageinfo.h"

void registerBatteryPercentageInfoMetaType()
{
    qRegisterMetaType<BatteryPercentageInfo>("BatteryPercentageInfo");
    qDBusRegisterMetaType<BatteryPercentageInfo>();
}
