// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "batterypercentageinfo.h"

void registerBatteryPercentageInfoMetaType()
{
    qRegisterMetaType<BatteryPercentageInfo>("BatteryPercentageInfo");
    qDBusRegisterMetaType<BatteryPercentageInfo>();
}
