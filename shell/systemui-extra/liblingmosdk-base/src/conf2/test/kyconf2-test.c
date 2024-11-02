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
 * Authors: tianshaoshuai <tianshaoshuai@kylinos.cn>
 *
 */

#include "../api/libkysettings.h"
#include <stdio.h>
#include <unistd.h>
#include <glib-2.0/glib.h>

// 键值改变信号回调函数
static void on_key_changed(KSettings *setting, char *key, void *user_data)
{
    char *value = kdk_conf2_get_value(setting, key);
    printf("%s change to %s\n", key, value);
}

// 键值改变信号回调函数
static void on_reload(KSettings *setting, void *user_data)
{
    printf("reload signal\n");
}

int main(int argc, char *argv[])
{
    KSettings *setting = kdk_conf2_new("test.notice", NULL);
    if (NULL == setting)
    {
        printf("get handle failed\n");
        return 0;
    }

    /**
     * 只有已连接切读过一次配置时才能接收到该信号
     * 接收信号需要开启主程序的消息循环
     */
    // kdk_conf2_connect_signal(setting, "changed::int", on_key_changed, NULL);

    char *schema_summary = kdk_conf2_get_schema_summary(setting);
    if (schema_summary)
    {
        printf("test.notice summary\t: %s\n", schema_summary);
        free(schema_summary);
    }

    char *schema_description = kdk_conf2_get_schema_desription(setting);
    if (schema_description)
    {
        printf("test.notice description\t: %s\n", schema_description);
        free(schema_description);
    }

    char *id = kdk_conf2_get_id(setting);
    if (id)
    {
        printf("id is\t: %s\n", id);
        free(id);
    }

    char *version = kdk_conf2_get_version(setting);
    if (version)
    {
        printf("version is\t: %s\n", version);
        free(version);
    }

    char **children = kdk_conf2_list_children(setting);
    if (children)
    {
        printf("children:\n");
        for (int i = 0; children[i]; i++)
        {
            printf("\t%s\n", children[i]);
            free(children[i]);
        }
        free(children);
    }

    char **keys = kdk_conf2_list_keys(setting);
    if (keys)
    {
        printf("keys:\n");
        for (int i = 0; keys[i]; i++)
        {
            printf("\t%s\n", keys[i]);
            free(keys[i]);
        }
        free(keys);
    }

    // 把childname替换为正确的子组名
    KSettings *child = kdk_conf2_get_child(setting, "proxy");
    if (child)
    {
        printf("get child :test.notice.proxy\n");
    }

    char *range = kdk_conf2_get_range(setting, "type");
    if (range)
    {
        printf("Type range is %s\n", range);
        free(range);
        range = NULL;
    }

    int success = 0;
    success = kdk_conf2_range_check(setting, "int", "65540");
    printf("value is %s\n", success ? "valid" : "invalid");

    {
        /**********通用读写**********/
        char *value = kdk_conf2_get_default_value(setting, "int");
        if (NULL != value)
        {
            printf("Int default value: %s\n", value);
            free(value);
        }

        value = kdk_conf2_get_value(setting, "int");
        if (NULL != value)
        {
            printf("Int value: %s\n", value);
            free(value);
        }

        success = kdk_conf2_set_value(setting, "int", "8081");
        if (0 == success)
        {
            printf("set int failed\n");
        }

        /**********枚举读写**********/
        int e = kdk_conf2_get_default_enum(setting, "type");
        printf("Type default value is %d\n", e);

        e = kdk_conf2_get_enum(setting, "type");
        printf("Type value is %d\n", e);

        success = kdk_conf2_set_enum(setting, "type", 0x2);
        if (0 == success)
        {
            printf("set type failed\n");
        }

        /**********整形读写**********/
        int i = kdk_conf2_get_default_int(setting, "int");
        printf("Int default value is %d\n", i);

        i = kdk_conf2_get_int(setting, "int");
        printf("Int value is %d\n", i);

        success = kdk_conf2_set_int(setting, "int", i + 1);
        if (0 == success)
        {
            printf("set int failed\n");
        }

        /**********长整形读写**********/
        long l = kdk_conf2_get_default_int64(setting, "long");
        printf("Long default value is %d\n", l);

        l = kdk_conf2_get_int64(setting, "long");
        printf("Long value is %d\n", l);

        success = kdk_conf2_set_int64(setting, "long", l + 1);
        if (0 == success)
        {
            printf("set long failed\n");
        }

        /**********无符号整形读写**********/
        unsigned int u = kdk_conf2_get_default_uint(setting, "uint");
        printf("Uint default value is %d\n", u);

        u = kdk_conf2_get_uint(setting, "uint");
        printf("Uint value is %d\n", u);

        success = kdk_conf2_set_uint(setting, "uint", u + 1);
        if (0 == success)
        {
            printf("set uint failed\n");
        }

        /**********无符号长整形读写**********/
        unsigned long t = kdk_conf2_get_default_uint64(setting, "ulong");
        printf("Ulong default value is %d\n", t);

        t = kdk_conf2_get_uint64(setting, "ulong");
        printf("Ulong value is %d\n", t);

        success = kdk_conf2_set_uint64(setting, "ulong", t + 1);
        if (0 == success)
        {
            printf("set ulong failed\n");
        }

        /**********布尔型读写**********/
        int b = kdk_conf2_get_default_boolean(setting, "enable");
        printf("Enable default value is %s\n", b ? "true" : "false");

        b = kdk_conf2_get_boolean(setting, "enable");
        printf("Enable value is %s\n", b ? "true" : "false");

        success = kdk_conf2_set_boolean(setting, "enable", !b);
        if (0 == success)
        {
            printf("set enable failed\n");
        }

        /**********浮点型读写**********/
        double d = kdk_conf2_get_default_double(setting, "transparency");
        printf("Transparency default value is %lf\n", d);

        d = kdk_conf2_get_double(setting, "transparency");
        printf("Transparency value is %lf\n", d);

        success = kdk_conf2_set_double(setting, "transparency", d + 1);
        if (0 == success)
        {
            printf("Set transparency failed\n");
        }

        /**********字符串读写**********/
        char *s = kdk_conf2_get_default_string(setting, "name");
        printf("Name default value is %s\n", s);

        s = kdk_conf2_get_string(setting, "name");
        printf("Name value is %s\n", s);

        success = kdk_conf2_set_string(setting, "name", "dsefe");
        if (0 == success)
        {
            printf("Set name failed\n");
        }

        /**********字符串列表读写**********/
        char **sv = kdk_conf2_get_default_strv(setting, "list");
        if (sv)
        {
            printf("List default value :\n");
            for (int i = 0; sv[i]; i++)
            {
                printf("\t%s\n", sv[i]);
            }
            g_strfreev(sv);
        }

        sv = kdk_conf2_get_strv(setting, "list");
        if (sv)
        {
            printf("List value :\n");
            for (int i = 0; sv[i]; i++)
            {
                printf("\t%s\n", sv[i]);
            }
            g_strfreev(sv);
        }
        const char *v[] = {"CESI仿宋-GB13000", "CESI_FS_GB13000", NULL};
        success = kdk_conf2_set_strv(setting, "list", v);
        if (0 == success)
        {
            printf("Set list failed\n");
        }
    }

    char *description = kdk_conf2_get_descrition(setting, "int");
    if (description)
    {
        printf("Int description: %s\n", description);
        free(description);
    }

    char *summary = kdk_conf2_get_summary(setting, "int");
    if (summary)
    {
        printf("Int summary: %s\n", summary);
        free(summary);
    }

    char *type = kdk_conf2_get_type(setting, "int");
    if (type)
    {
        printf("Int type: %s\n", type);
        free(type);
    }

    kdk_conf2_reset(setting, "int");

    int writable = kdk_conf2_is_writable(setting, "int");
    printf("Int is %s\n", writable ? "writable" : "not writable");

    // kdk_conf2_connect_signal(setting, "reload", on_reload, NULL);
    kdk_conf2_reload();

    char **schemas = kdk_conf2_list_schemas("test", NULL);
    if (schemas)
    {
        printf("test schemas:\n");
        for (int i = 0; schemas[i]; i++)
        {
            printf("\t%s\n", schemas[i]);
            free(schemas[i]);
        }
        free(schemas);
    }

    KSettings *externds_setting = kdk_conf2_new_extends_id("test.notice", "test.notice.tss", NULL);
    if (externds_setting)
    {
        printf("kdk_conf2_new_extends_id successed\n");
    }

    int is_schema = kdk_conf2_is_schema("control-center.notice", NULL);
    printf("control-center.notice is a %s shcema\n", is_schema ? "valid" : "invalid");

    // 注销句柄
    kdk_conf2_ksettings_destroy(setting);

    return 0;
}
