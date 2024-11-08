/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include "stdio.h"
#include "../libkybattery.h"
#include <stdlib.h>
int main()
{
    float percentage = kdk_battery_get_soc();
    printf("percentage = %0.2f\n", percentage);

    BatteryChargeState state = kdk_battery_get_charge_state();
    printf("state = %d\n", state);

    BatteryHealthState health_state = kdk_battery_get_health_state();
    printf("health state = %d\n", health_state);

    BatteryPluggedType type = kdk_battery_get_plugged_type();
    printf("type = %d\n", type);

    float voltage = kdk_battery_get_voltage();
    printf("voltage = %0.3f\n", voltage);

    char* model = kdk_battery_get_technology();
    printf("model = %s\n", model);

    float temperature = kdk_battery_get_temperature();
    printf("temperature = %0.1f\n", temperature);

    bool is_present = kdk_battery_is_present();
    printf("is_present : %s\n", is_present ? "true" : "false");

    int level = kdk_battery_get_capacity_level();
    printf("level = %d\n", level);

    return 0;
}
