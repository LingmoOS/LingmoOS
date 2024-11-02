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

#ifndef KSETTINGSSCHEMA_H
#define KSETTINGSSCHEMA_H

#include <gio/gio.h>
#include <syslog.h>

#if __cplusplus
extern "C"
{
#endif
    typedef struct _AppData
    {
        char *name;
        char *default_version;
    } AppData;

    typedef struct _VersionData
    {
        char *name;
        char **compatible;
    } VersionData;

    typedef struct _SchemaData
    {
        GHashTable *values;
        GArray *children;
        char *id;
        char *version;
        char *permission;
        char *description;
        char *summary;
        char *extends;
    } SchemaData;

    typedef struct _SchemaData KSettingsSchema;

    extern GHashTable *schemas_table; //用于缓存配置数据

    /**
     * @brief 在hash表中查找id，并返回句柄
     * 
     * @param id 配置id
     * @param version 配置版本
     * @return KSettingsSchema* 配置句柄 
     */
    KSettingsSchema *kdk_conf2_schema_table_lookup(const char *id, const char *version);

    /**
     * @brief 重新读取数据库，更新hash表
     * 
     * @param id 
     * @param version 
     * @return KSettingsSchema* 
     */
    KSettingsSchema *kdk_conf2_schema_reload(const char *id, const char *version);

    /**
     * @brief 销毁句柄
     * 
     * @param schema 配置句柄
     */
    void kdk_conf2_schema_destroy(KSettingsSchema *schema);

    /**
     * @brief 获取配置id
     * 
     * @param schema 配置句柄
     * @return char* 配置id
     */
    char *kdk_conf2_schema_get_id(KSettingsSchema *schema);

    /**
     * @brief 获取配置版本
     * 
     * @param schema 配置句柄
     * @return char* 配置版本
     */
    char *kdk_conf2_schema_get_version(KSettingsSchema *schema);

    /**
     * @brief 获取键的数据
     * 
     * @param schema 配置句柄
     * @param key 键名
     * @return GHashTable* 存储键所有数据的hash表，包含默认值，简述、详述等 
     */
    GHashTable *kdk_conf2_schema_get_key(KSettingsSchema *schema, const char *key);

    /**
     * @brief 判断配置句柄是否存在某个key
     * 
     * @param schema 配置句柄
     * @return int 1 存在，0不存在
     */
    int kdk_conf2_schema_has_key(KSettingsSchema *schema);

    /**
     * @brief 获取配置句柄所有key
     * 
     * @param schema 配置句柄
     * @return char** key的列表
     */
    char **kdk_conf2_schema_list_keys(KSettingsSchema *schema);

    /**
     * @brief 获取配置句柄的所有子组名
     * 
     * @param schema 配置句柄
     * @return char** 组名列表 
     */
    char **kdk_conf2_schema_list_children(KSettingsSchema *schema);

    /**
     * @brief 判断配置句柄中是否存在子组
     * 
     * @param schema 配置句柄
     * @param name 子组名
     * @return int 1-存在，0-不存在
     */
    int kdk_conf2_schema_has_child(KSettingsSchema *schema, const char *name);

    /**
     * @brief 获取键值类型
     * 
     * @param key 键的hash表
     * @return char* 类型字符串
     */
    char *kdk_conf2_schema_key_value_type(GHashTable *key);

    /**
     * @brief 获取键当前值
     * 
     * @param key 键的hash表
     * @return char* 键当前值
     */
    char *kdk_conf2_schema_key_get_value(GHashTable *key);

    /**
     * @brief 获取键的默认值
     * 
     * @param key 键的hash表
     * @return char* 键的默认值
     */
    char *kdk_conf2_schema_key_get_default_value(GHashTable *key);

    /**
     * @brief 获取键取值范围
     * 
     * @param key 键的hash表
     * @return char* 键的取值范围，如果是数值类型返回字符串"min,max",如果是枚举返回字符串"{nick : value}"
     */
    char *kdk_conf2_schema_key_get_range(GHashTable *key);

    /**
     * @brief 检测目标值是否在取值范围内
     * 
     * @param key 键的hash表
     * @param value 目标值
     * @return int 1-在取值范围；0-不在取值范围
     */
    int kdk_conf2_schema_key_range_check(GHashTable *key, const char *value);

    /**
     * @brief 获取键名
     * 
     * @param key 键的hash表
     * @return char* 键名
     */
    char *kdk_conf2_schema_key_get_name(GHashTable *key);

    /**
     * @brief 获取键的简述
     * 
     * @param key 键的hash表
     * @return char* 简述
     */
    char *kdk_conf2_schema_key_get_summary(GHashTable *key);

    /**
     * @brief 获取键的详述
     * 
     * @param key 键的hash表
     * @return char* 详述
     */
    char *kdk_conf2_schema_key_get_description(GHashTable *key);

    /**
     * @brief 获取键的权限类型
     * 
     * @param key 键的hash表
     * @return char* 键的权限
     */
    char *kdk_conf2_schema_key_get_permission(GHashTable *key);

    /**
     * @brief 更新数据缓存schemas_table
     * 
     */
    void kdk_conf2_schema_update_schemas_table();

    /**
     * @brief 设置键的当前值
     * 
     * @param key 键的hash表
     * @param value 目标值
     */
    void kdk_conf2_schema_key_set_value(GHashTable *key, char *value);

    /**
     * @brief 清理键的当前值
     * 
     * @param key 键的hash表
     */
    void kdk_conf2_schema_key_reset(GHashTable *key);

#if __cplusplus
}
#endif

#endif // KSETTINGSSCHEMA_H
