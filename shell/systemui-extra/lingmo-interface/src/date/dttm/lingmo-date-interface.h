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


#ifndef __LINGMODATEINTERFACE_H__
#define __LINGMODATEINTERFACE_H__

#include<stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

/*
 * Modify the time zone.
 * @location is the time zone.Referring to the file "/usr/share/zoneinfo/zone_utc".
 * e.g. gchar *location="(UTC+08:00) 北京，重庆，香港特别行政区，乌鲁木齐";
 */
void lingmo_date_dt_settz(char *location);

/*
 * Change the system date and time.
 * @Parameter represents the date and time you want to set.
 */
void lingmo_date_dt_chgdt(unsigned year,
                         unsigned month,
                         unsigned day,
                         unsigned hour,
                         unsigned minute,
                         unsigned second);

/*
 * Whether to enable network time synchronization.
 * @timesetting can be equal to 1 or 0.
 */
void lingmo_date_dt_setnetsync(int timesetting);

/*
 * Whether to enable time format.
 */
void lingmo_date_dt_sethrfmt(bool value);

#ifdef __cplusplus
}
#endif

#endif


