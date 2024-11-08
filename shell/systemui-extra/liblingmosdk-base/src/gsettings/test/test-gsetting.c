/*
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
 * Authors: shaozhimin <shaozhimin@kylinos.cn>
 *
 */

#include "../libkygsetting.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>

int main()
{
	int ret = kdk_settings_reset("org.gnome.evolution-data-server.calendar", "notify-custom-snooze-minutes");

	double a[] = {0.0,2.0};  
	size_t s = 2;
 	// double **s = a;
	
	ret = kdk_gsettings_set("org.onboard.window", "docking-aspect-change-range", "ad", a, s);	//当类型为数组时，请输入数组大小，size_t
	printf("%d\n", ret);
	ret = kdk_settings_set_string("org.gnome.evolution", "version", "3.36.1");
	ret = kdk_settings_set_int("org.lingmo.audio", "output-volume", 67);
	char* res = kdk_settings_get_string("org.gnome.evolution", "version");
	printf("res = %s\n", res);

	int key = kdk_settings_get_int("org.lingmo.audio", "output-volume");
	printf("key = %d\n", key);
	bool enabled;
	GVariantIter iter;
	kdk_gsettings_get("org.lingmo.control-center.notice", "blacklist", "as", &iter);
	printf("enable = %d\n", enabled);
	// printf("value = %d\n", value);
	// for (int i = 0; value[i]; i++)
    // {
    //     printf("[%d]Get disk info error.\n", value[i]);
	// }

	double dde = kdk_settings_get_double("org.lingmo.SettingsDaemon.plugins.xsettings", "scaling-factor");
	printf("dde = %0.1f\n", dde);
	return 0;
} 
