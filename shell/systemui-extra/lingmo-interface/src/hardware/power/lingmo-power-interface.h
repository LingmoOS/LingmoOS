/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */


#ifndef __LINGMOPOWERINTERFACE_H__
#define __LINGMOPOWERINTERFACE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//Initialize the DBus Communication
bool InitDBusPower(void);

//Delete the DBus Communication
bool DeInitDBusPower(void);
/*
 * Sleep timeout computer when on battery
 */
void lingmo_hardware_power_sleepcomputerbattery(const int in_arg);

/*
 * Sleep timeout computer when on AC
 */
void lingmo_hardware_power_sleepcomputerac(const int in_arg);

/*
 * Sleep timeout display when on battery
 */
void lingmo_hardware_power_sleepdisplaybattery(const int in_arg);

/*
 * Sleep timeout display when on AC
 */
void lingmo_hardware_power_sleepdisplayac(const int in_arg);

/*
 * Display options for the notification icon.
 * Possible values are:
 * @in_arg is equal to 0, "always";
 * @in_arg is equal to 1, "present";
 * @in_arg is equal to 2, "charge";
 * @in_arg is equal to 3, "low";
 * @in_arg is equal to 4, "critical";
 * @in_arg is equal to 5, "never".
 */
void lingmo_hardware_power_iconpolicy(const int in_arg);

#ifdef __cplusplus
}
#endif

#endif













