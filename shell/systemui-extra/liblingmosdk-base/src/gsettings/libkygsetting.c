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

#include "libkygsetting.h"
#include <stdio.h>
#include <gio/gio.h>
#include <gio/gsettings.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib-object.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

gboolean schema_key_is_exist(char *schema_id, char *name)
{
    gboolean ret;
    GSettingsSchema *schema;

    if(schema_id == NULL || name == NULL) {
        return FALSE;
    }

    schema = g_settings_schema_source_lookup(g_settings_schema_source_get_default(),schema_id, FALSE);
    if (schema == NULL) {
        return FALSE;
    }

    ret = g_settings_schema_has_key (schema, name);
    if(!ret) {
        return FALSE;
    }
    return TRUE;

}

int kdk_gsettings_set(const char *schema_id, const char *key, const char *format, ...)
{
    int ret = 0;
    GSettings *settings;
    GVariant *value;
    va_list ap;
    if(schema_id == NULL || key == NULL || format == NULL) {
        printf("incorrect input");
        return 0;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return 0;
    }
    settings = g_settings_new(schema_id);
    va_start(ap, format);

    if (strstr(format, "as"))
    {
        char **variant;
        GArray *as;

        variant = va_arg(ap, char **);
        va_end(ap);

        as = g_array_new(TRUE, TRUE, sizeof(char **));
        int i = 0;
        for (i; variant[i]; i++)
        {
        }
        variant[i - 1] = NULL;
        for (int j = 0; variant[j]; j++)
        {
            g_array_append_val(as, variant[j]);
        }

        ret = g_settings_set_strv(settings, key, (const char **)as->data);
        g_array_free(as, TRUE);
    }
    else if (strstr(format, "ai"))
    {
        GVariantBuilder *builder = NULL;
        GVariant *variant;

        int *res;
        res = va_arg(ap, int *);
        size_t size = va_arg(ap, size_t);
        
        variant = g_variant_new_fixed_array(G_VARIANT_TYPE_INT32, res, size, sizeof(res[0]));
        va_end(ap);

        ret = g_settings_set_value(settings, key, variant);
    }
    else if (strstr(format, "ad"))
    {
        GVariant *variant;

        double *res;
        res = va_arg(ap, double*);

        int size = va_arg(ap, int);
        variant = g_variant_new_fixed_array(G_VARIANT_TYPE_DOUBLE, res, size, sizeof(res[0]));
        va_end(ap);

        ret = g_settings_set_value(settings, key, variant);
    }
    else
    {
        value = g_variant_new_va(format, NULL, &ap);
        va_end(ap);

        ret = g_settings_set_value(settings, key, g_steal_pointer(&value));
    }

    g_settings_sync();
    g_object_unref(settings);
    return ret;
}

int kdk_settings_reset(const char *schema_id, const char *key)
{
    GSettings *settings;
    if(schema_id == NULL || key == NULL) {
        printf("incorrect input");
        return 0;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return 0;
    }
    settings = g_settings_new(schema_id);
    g_settings_reset(settings, key);
    g_settings_sync();
    g_object_unref(settings);
    return 1;
}

int kdk_settings_set_string(const char *schema_id, const char *key, const char *value)
{
    GSettings *settings;
    if(schema_id == NULL || key == NULL || value == NULL) {
        printf("incorrect input");
        return 0;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return 0;
    }
    settings = g_settings_new(schema_id);
    g_settings_set_string(settings, key, value);
    g_settings_sync();
    g_object_unref(settings);
    return 1;
}

int kdk_settings_set_int(const char *schema_id, const char *key, int value)
{
    GSettings *settings;
    if(schema_id == NULL || key == NULL || value == NULL) {
        printf("incorrect input");
        return 0;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return 0;
    }
    settings = g_settings_new(schema_id);
    g_settings_set_int(settings, key, value);
    g_settings_sync();
    g_object_unref(settings);
    return 1;
}

void *kdk_gsettings_get(const char *schema_id, const char *key, const char *format, ...)
{
    GSettings *settings;
    GVariant *value;
    va_list ap;

    if(schema_id == NULL || key == NULL || format == NULL) {
        printf("incorrect input");
        return NULL;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return NULL;
    }
    settings = g_settings_new(schema_id);

    value = g_settings_get_value(settings, key);

    va_start(ap, format);
    g_variant_get_va(value, format, NULL, &ap);
    va_end(ap);

    g_variant_unref(value);

    return NULL;
}

char *kdk_settings_get_string(const char *schema_id, const char *key)
{
    GSettings *settings;
    char *current_setting;
    if(schema_id == NULL || key == NULL) {
        printf("incorrect input");
        return NULL;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return NULL;
    }
    settings = g_settings_new(schema_id);
    current_setting = g_settings_get_string(settings, key);
    return current_setting;
}

int kdk_settings_get_int(const char *schema_id, const char *key)
{
    GSettings *settings;
    int current_setting;
    if(schema_id == NULL || key == NULL) {
        printf("incorrect input");
        return 0;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return 0;
    }
    settings = g_settings_new(schema_id);
    current_setting = g_settings_get_int(settings, key);
    return current_setting;
}

double kdk_settings_get_double(const char *schema_id, const char *key)
{
    GSettings *settings;
    double current_setting;
    if(schema_id == NULL || key == NULL) {
        printf("incorrect input");
        return 0;
    }

    if(!schema_key_is_exist(schema_id,key)) {
        printf("%s does not have a key named %s exist", schema_id, key);
        return 0;
    }
    settings = g_settings_new(schema_id);
    current_setting = g_settings_get_double(settings, key);
    return current_setting;
}
