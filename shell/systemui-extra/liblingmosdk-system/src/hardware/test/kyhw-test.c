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

#include "../libkyhw.h"
#include <stdio.h>
#include <string.h>

static void _print_hw(struct HWInfo *hw)
{
    struct HWInfo *tmp = hw;
    while (tmp)
    {
        printf("\tmodel : %s\n", tmp->model);
        printf("\t\tvendor : %s\n", tmp->vendor);
        printf("\t\tdevice : %s\n", tmp->device);
        printf("\t\tdriver : %s\n", tmp->driver);
        printf("\t\trevision : %s\n", tmp->revision);
        printf("\t\tdevicenum : %s\n", tmp->devicenum);
        printf("\t\tclock : %s\n", tmp->clock);
        printf("\t\twidth : %s\n", tmp->width);
        tmp = tmp->next;
    }
}

static void Traverse_hardware()
{
    struct HWInfo *keyboard = kdk_hw_get_hwinfo(3);
    printf("KeyBoard Info:\n");
    _print_hw(keyboard);
    kdk_hw_free_hw(keyboard);

    struct HWInfo *mouse = kdk_hw_get_hwinfo(5);
    printf("Mouse Info:\n");
    _print_hw(mouse);
    kdk_hw_free_hw(mouse);

    struct HWInfo *bound = kdk_hw_get_hwinfo(15);
    printf("Bound Info:\n");
    _print_hw(bound);
    kdk_hw_free_hw(bound);

    struct HWInfo *cdrom = kdk_hw_get_hwinfo(23);
    printf("CDRom Info:\n");
    _print_hw(cdrom);
    kdk_hw_free_hw(cdrom);

    struct HWInfo *camera = kdk_hw_get_hwinfo(27);
    printf("Camera Info:\n");
    _print_hw(camera);
    kdk_hw_free_hw(camera);
}

static void Traverse_power()
{
    struct Power * power = kdk_hw_get_powerinfo();
    printf("on_battery : %s\n", power->on_battery ? "true" : "false");
    printf("lid_is_closed : %s\n", power->lid_is_closed ? "true" : "false");
    printf("lid_is_present : %s\n", power->lid_is_present ? "true" : "false");
    printf("daemon version : %s\n", power->daemon_version);
    printf("critical action : %s\n", power->critical_action);

    struct power_device *tmp = power->devices;
    while (tmp)
    {
        printf("name : %s\n", tmp->name);
        printf("\tnative path : %s\n", tmp->native_path);
        printf("\tpower supply : %s\n", tmp->power_supply ? "true" : "false");
        printf("\tupdated : %s\n", tmp->updated);
        printf("\thas history : %s\n", tmp->has_history ? "true" : "false");
        printf("\thas statistics : %s\n", tmp->has_statistics ? "true" : "false");
        printf("\tis persent: %s\n", tmp->is_persent ? "true" : "false");
        printf("\tis rechargeable : %s\n", tmp->is_rechargeable ? "true" : "false");
        printf("\tstate : %s\n", tmp->state);
        printf("\twaring level : %s\n", tmp->warning_level);
        printf("\tenergy : %0.2f\n", tmp->energy);
        printf("\tenergy empty : %0.2f\n", tmp->energy_empty);
        printf("\tenergy full: %0.2f\n", tmp->energy_full);
        printf("\tenergy full design : %0.2f\n", tmp->energy_full_design);
        printf("\tenergy rate : %0.2f\n", tmp->energy_rate);
        printf("\tvoltage : %0.2f\n", tmp->voltage);
        printf("\ttime to empty : %ld\n", tmp->time_to_empty);
        printf("\ttime to full : %ld\n", tmp->time_to_full);
        printf("\tpercentage : %0.2f\n", tmp->percentage);
        printf("\ttemperature : %0.2f\n", tmp->temperature);
        printf("\tcapacity : %0.2f\n", tmp->capacity);
        printf("\ttechnology : %s\n", tmp->technology);
        printf("\tonline : %s\n", tmp->online ? "true" : "false");
        printf("\ticon name : %s\n", tmp->icon_name);
        printf("\tmodel : %s\n", tmp->model);
        printf("\tbattery_level : %d\n", tmp->battery_level);
        printf("\ttype : %d\n", tmp->type);
        tmp = tmp->next;
    }
    kdk_hw_free_power_info(power);
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("please use like >> kyhwinfo-test --[target]\n");
        printf("hardware");
        printf("\tpower");
        printf("\n");
        return 0;
    }
    if (0 == strcmp(argv[1], "--hardware"))
        Traverse_hardware();
    if (0 == strcmp(argv[1], "--power"))
        Traverse_power();
}
