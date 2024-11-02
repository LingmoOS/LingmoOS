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


#include <stdio.h>
#include "lingmo-gsettings-set.h"
#include <gio/gio.h>
#include <stdbool.h>

//Creates a new GSsetting object with the schema specified by schema_id
GSettings *gsettings_get_schemaID(const gchar *schemaID)
{
	
  GSettings *settings = g_settings_new(schemaID);

  return settings;
}

//Sets int_key in settings to value
bool gsettings_set_int(GSettings *settings, const gchar *key, gint value)
{

	bool ret=g_settings_set_int(settings, key, value);
	return ret;

}

//Sets boolean_key in settings to value
bool gsettings_set_boolean(GSettings *settings, const gchar *key, gboolean value)
{
	bool ret=g_settings_set_boolean(settings,key,value);
	return ret;

}

//Sets double_key in settings to value
bool gsettings_set_double(GSettings *settings, const gchar *key, gdouble value)
{

	bool ret=g_settings_set_double(settings, key, value);
	return ret;

}

//Sets string_key in settings to value
bool gsettings_set_string(GSettings *settings, const gchar *key, const gchar *value)
{
	bool ret=g_settings_set_string(settings, key, value);
	return ret;

}

//Sets enum_key in settings to value
bool gsettings_set_enum(GSettings *settings, const gchar *key, gint value)
{

	bool ret=g_settings_set_enum(settings, key, value);
	return ret;

}

//Decreases the reference count of setting
void Dec_ref(GSettings *setting)
{
    g_object_unref(setting);
}

//fix library-not-linked-against-libc by lintian
void fun()
{
	printf("fun\n");
}
