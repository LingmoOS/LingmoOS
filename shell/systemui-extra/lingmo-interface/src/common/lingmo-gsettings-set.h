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

#ifndef __LINGMOGSETTINGSSET_H__
#define __LINGMOGSETTINGSSET_H__

#include <stdio.h>
#include <gio/gio.h>
#include <stdbool.h>
#ifdef  __cplusplus
extern "C" {
#endif

//Creates a new GSsetting object with the schema specified by schema_id
GSettings *gsettings_get_schemaID(const gchar *schemaID);

//Sets int_key in settings to value
bool gsettings_set_int(GSettings *settings, const gchar *key, gint value);

//Sets boolean_key in settings to value
bool gsettings_set_boolean(GSettings *settings, const gchar *key, gboolean value);

//Sets double_key in settings to value
bool gsettings_set_double(GSettings *settings, const gchar *key, gdouble value);

//Sets string_key in settings to value
bool gsettings_set_string(GSettings *settings, const gchar *key, const gchar *value);

//Sets enum_key in settings to value
bool gsettings_set_enum(GSettings *settings, const gchar *key, gint value);

//Decreases the reference count of setting
void Dec_ref(GSettings *setting);

#ifdef  __cplusplus
}
#endif

#endif
