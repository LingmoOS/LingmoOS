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

#include "ksettingsschema.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <pwd.h>

#include "libkylog.h"

/**
 * @brief 嵌套哈希表，
 * 表层 key: AppData* value: 版本号哈希表GHashTable*
 * 版本号哈希表 key VersionData* 项目哈希表GHashTable*
 * 项目哈希表 key 项目id char*    配置详情KSettingsSchema*
 * {AppData* ：{VersionData* ：{char* : KSettingsScheam*}}}
 */
GHashTable *schemas_table = NULL;
/*************************hash函数*****************************/

guint data_hash(gconstpointer data)
{
    return g_str_hash(((AppData *)data)->name);
}

gboolean data_equal(gconstpointer a, gconstpointer b)
{
    return 0 == g_strcmp0(((AppData *)a)->name, ((AppData *)b)->name);
}

void app_data_destroy(gpointer data)
{
    if (NULL == data)
        return;
    AppData *tmp = (AppData *)data;
    g_free(tmp->name);
    g_free(tmp->default_version);
    g_free(tmp);
}

void version_data_destroy(gpointer data)
{
    if (NULL == data)
        return;
    VersionData *tmp = (VersionData *)data;
    g_free(tmp->name);
    g_strfreev(tmp->compatible);
    g_free(tmp);
}

void schema_data_destroy(gpointer data)
{
    if (NULL == data)
        return;
    KSettingsSchema *schema = (KSettingsSchema *)data;
    g_free(schema->id);
    g_free(schema->version);
    g_free(schema->permission);
    g_free(schema->description);
    g_free(schema->summary);
    g_free(schema->extends);
    if (schema->values)
    {
        g_hash_table_destroy(schema->values);
        schema->values = NULL;
    }
    if (schema->children)
    {
        g_array_free(schema->children, TRUE);
        schema->children = NULL;
    }
    g_free(schema);
}

static char **_split_string(const gchar *id, const char seq)
{
    if (NULL == id)
        return NULL;

    const gchar *p, *q;
    p = q = id;
    int count = 1;
    while (*p != '\0')
    {
        if (*p++ == seq)
            count++;
    }

    char **list = calloc(count + 1, sizeof(char *));
    if (NULL == list)
    {
        // TODO: write log
        return NULL;
    }

    int i = 0;
    p = id;
    while (*p != '\0')
    {
        if (*p == seq)
        {
            list[i] = calloc(p - q + 1, sizeof(char));
            if (NULL == list[i])
            {
                g_strfreev(list);
                return NULL;
            }
            strncpy(list[i++], q, p - q);
            q = p + 1;
        }
        p++;
    }
    list[i] = calloc(p - q + 1, sizeof(char));
    if (NULL == list[i])
    {
        g_strfreev(list);
        return NULL;
    }
    strncpy(list[i], q, p - q);

    return list;
}

/**********************************************读取数据库*****************************************************/

/************************************SQL回调函数*********************************************/
// 读取一行
static int _sql_call_get_columns(void *data, int argc, char **argv, char **azColName)
{
    *((char ***)data) = g_strdupv(argv);
    return 0;
}

// 读取多行
static int line_count = 0; // 行计数
static int _sql_call_get_lines(void *data, int argc, char **argv, char **azColName)
{
    char ***lines = *((char ****)data);
    char **columns = calloc(argc + 1, sizeof(char *));
    for (int i = 0; i < argc; i++)
    {
        columns[i] = g_strdup(argv[i]);
    }
    char ***tmp = realloc(lines, sizeof(char **) * (line_count + 2));
    if (NULL == tmp)
    {
        line_count = 0;
        for (int i = 0; i < line_count; i++)
        {
            for (int j = 0; j < argc; j++)
            {
                free(lines[i][j]);
            }
            free(lines[i]);
        }
        free(lines);
        lines = NULL;
        return 1;
    }
    lines = tmp;
    lines[line_count++] = columns;
    lines[line_count] = NULL;
    *((char ****)data) = lines;
    return 0;
}

// 读取一个组件key和child
static int _sql_call_get_schema(void *data, int argc, char **argv, char **azColName)
{
    KSettingsSchema *schema = (KSettingsSchema *)data;

    char *node_name = argv[2];
    char *node_type = argv[3];
    char *permission = argv[4];
    char *description = argv[5];
    char *summary = argv[6];
    char *value_type = argv[8];
    char *custom_value = argv[9];
    char *default_value = argv[10];
    char *value_range = argv[11];
    char *extends = argv[12];

    if (0 == strcmp(node_type, "key"))
    {
        // hash表  s：hash（s:s）
        if (NULL == schema->values)
            schema->values = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_hash_table_destroy);
        // hash表 s:s
        GHashTable *item = g_hash_table_new(g_str_hash, g_str_equal);
        if (NULL == item)
            goto err;
        if (node_name)
            g_hash_table_insert(item, "node_name", g_strdup(node_name)); // 键名
        if (permission)
            g_hash_table_insert(item, "permission", g_strdup(permission)); // 读写权限
        if (description)
            g_hash_table_insert(item, "description", g_strdup(description)); // 详细描述
        if (summary)
            g_hash_table_insert(item, "summary", g_strdup(summary)); // 概要
        if (value_type)
            g_hash_table_insert(item, "value_type", g_strdup(value_type)); // 类型
        if (custom_value)
            g_hash_table_insert(item, "custom_value", g_strdup(custom_value)); // 键值
        if (default_value)
            g_hash_table_insert(item, "default_value", g_strdup(default_value)); // 默认值
        if (value_range)
            g_hash_table_insert(item, "range", g_strdup(value_range)); // 取值范围

        g_hash_table_insert(schema->values, g_strdup(node_name), item);
    }
    else if (0 == strcmp(node_type, "schema"))
    {
        if (NULL == schema->children)
        {
            schema->children = g_array_new(FALSE, FALSE, sizeof(char *));
        }
        char *tmp = g_strdup(node_name);
        g_array_append_val(schema->children, tmp);
    }
    return 0;
err:
    return 1;
}
/*SQL回调函数*/

/************************************SQL读取函数*********************************************/

// 读取一个组件
static KSettingsSchema *_get_schema(const char *id, const char *version)
{
    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :Get user name failed! use root config\n", __func__);
        strcpy(user_name, "root");
    }
    else
    {
        strcpy(user_name, pw->pw_name);
        endpwent();
    }

    KSettingsSchema *schema = NULL;

    char db_file[PATH_MAX];
    if (0 == strcmp(user_name, "root"))
        sprintf(db_file, "/root/.config/lingmo-config/user.db");
    else
        sprintf(db_file, "/home/%s/.config/lingmo-config/user.db", user_name);
    // syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :open %s \n", __func__, db_file);

    char **id_list = _split_string(id, '.');
    if (!id_list)
        return NULL;

    // 读取数据库数据
    sqlite3 *db = NULL;
    char *err_msg = NULL;

    int rc = sqlite3_open(db_file, &db);
    if (rc != SQLITE_OK)
    {
        if ((0 == strcmp(db_file, "/root/.config/lingmo-config/user.db")) ||
            (0 == strcmp(db_file, "/home/lightdm/.config/lingmo-config/user.db")) ||
            (0 == strcmp(db_file, "/home/messagebus/.config/lingmo-config/user.db")))
        {
            syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :open %s failed! %s try to open /etc/lingmo-config/user.db.\n", __func__, db_file, sqlite3_errmsg(db));
            rc = sqlite3_open("/etc/lingmo-config/user.db", &db);
            if (rc != SQLITE_OK)
            {
                syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :open /etc/lingmo-config/user.db failed. %s\n", __func__, sqlite3_errmsg(db));
                g_strfreev(id_list);
                return NULL;
            }
        }
        else
            return NULL;
    }

    rc = sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :无法开始事务: %s\n", __func__, err_msg);
        goto out;
    }

    // 查询数据
    int app_id = 0, version_id = 0, parent_id = 0;
    char sql[1024];
    sprintf(sql, "SELECT * FROM app WHERE app_name = '%s'", id_list[0]);
    char **columns = NULL;
    rc = sqlite3_exec(db, sql, _sql_call_get_columns, &columns, &err_msg);
    if (rc != SQLITE_OK || NULL == columns)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :查询应用%s失败: %s\n", __func__, id_list[0], err_msg);
        goto out;
    }
    app_id = atoi(columns[0]);
    if (NULL == version)
        version = columns[2];
    g_strfreev(columns);
    sprintf(sql, "SELECT * FROM version WHERE app_id = %d AND version = '%s'", app_id, version);

    rc = sqlite3_exec(db, sql, _sql_call_get_columns, &columns, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :查询%s版本号%s失败: %s\n", __func__, version, err_msg);
        goto out;
    }
    version_id = atoi(columns[0]);
    g_strfreev(columns);

    int i = 1;
    while (id_list[i])
    {
        sprintf(sql, "SELECT * FROM configures WHERE version_id = %d AND parent = %d AND node_name = '%s' AND node_type = 'schema'",
                version_id, parent_id, id_list[i++]);

        rc = sqlite3_exec(db, sql, _sql_call_get_columns, &columns, &err_msg);
        if (rc != SQLITE_OK)
        {
            syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :查询组件%s失败: %s\n", __func__, id_list[i], err_msg);
            goto out;
        }
        parent_id = atoi(columns[0]);
        g_strfreev(columns);
    }

    schema = (KSettingsSchema *)calloc(1, sizeof *schema);
    schema->id = strdup(id);
    schema->version = strdup(version);
    // 如果获取数据失败，毁掉函数_sql_call_get_key会释放result的内存并将result置空
    sprintf(sql, "SELECT * FROM configures WHERE version_id = %d AND parent = %d",
            version_id, parent_id);

    rc = sqlite3_exec(db, sql, _sql_call_get_schema, schema, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :查询失败: %s\n", __func__, err_msg);
        goto out;
    }

    rc = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :无法提交事务: %s\n", __func__, err_msg);
        goto out;
    }
    sqlite3_close(db);
    g_strfreev(id_list);
    return schema;
out:
    sqlite3_exec(db, "ROLLBACK", 0, 0, &err_msg);
    sqlite3_free(err_msg);
    sqlite3_close(db);

    g_strfreev(id_list);
    return schema;
}

static char *_get_db_file_path(char *buffer, size_t size)
{
    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :Get user name failed! use root config\n", __func__);
        return NULL;
    }
    else
    {
        strcpy(user_name, pw->pw_name);
        endpwent();
    }

    char db_file[PATH_MAX];
    if (0 == strcmp(user_name, "root"))
        snprintf(buffer, size, "/root/.config/lingmo-config/user.db");
    else
        snprintf(buffer, size, "/home/%s/.config/lingmo-config/user.db", user_name);
    return buffer;
}

/**
 * @brief 加载数据库所有数据到schemas_table
 * 流程1. 读取app数据表，获取app列表
 * 流程2. 读取version数据表，获取version列表
 * 流程3. 读取configures数据表将配置数据读取到内存中，根据app_id以及version_id将配置数据存储到schemas_table
 * 
 * 了解数据库表结构可查看概要设计或service/conf2Utils.py的_creat_db_file接口
 * 
 * schemas_table的结构看33行注释
 */
static void _get_schemas()
{
    char ***apps = NULL, ***versions = NULL, ***configures = NULL;

    uid_t uid = getuid(); // 获取当前进程的用户ID
    char user_name[128] = {0};
    struct passwd *pw = getpwuid(uid); // 获取用户ID对应的用户信息结构体
    if (NULL == pw)
    {
        syslog(LOG_DEBUG, "[lingmosdk-conf2] %s ->  :Get user name failed! use root config\n", __func__);
        strcpy(user_name, "root");
    }
    else
    {
        strcpy(user_name, pw->pw_name);
        endpwent();
    }

    char db_file[PATH_MAX];
    if (0 == strcmp(user_name, "root"))
        sprintf(db_file, "/root/.config/lingmo-config/user.db");
    else
        sprintf(db_file, "/home/%s/.config/lingmo-config/user.db", user_name);
    // syslog(LOG_DEBUG, "[lingmosdk-conf2] %s ->  :open %s \n", __func__, db_file);

    // 读取数据库数据
    sqlite3 *db = NULL;
    char *err_msg = NULL;

    // 打开数据库
    int rc = sqlite3_open(db_file, &db);
    if (rc != SQLITE_OK)
    {
        // 华为云项目中,systemd屏蔽用户目录;lighdm没有用户目录 所以读取etc下的数据
        if ((0 == strcmp(db_file, "/root/.config/lingmo-config/user.db")) ||
            (0 == strcmp(db_file, "/home/lightdm/.config/lingmo-config/user.db")) ||
            (0 == strcmp(db_file, "/home/messagebus/.config/lingmo-config/user.db")))
        {
            syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :open %s failed : %s! try to open /etc/lingmo-config/user.db.\n", __func__, db_file, sqlite3_errmsg(db));
            rc = sqlite3_open("/etc/lingmo-config/user.db", &db);
            if (rc != SQLITE_OK)
            {
                syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :open /etc/lingmo-config/user.db failed. %s\n", __func__, sqlite3_errmsg(db));
                return;
            }
        }
        else
        {
            syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :open %s failed : %s\n", __func__, db_file, sqlite3_errmsg(db));
            return;
        }
    }

    char sql[100];
    rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :无法开始事务: %s\n", __func__, err_msg);
        goto out;
    }

    strcpy(sql, "SELECT * FROM app");
    line_count = 0;
    rc = sqlite3_exec(db, sql, _sql_call_get_lines, &apps, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :查询应用名失败: %s\n", __func__, err_msg);
        goto out;
    }

    line_count = 0;
    sprintf(sql, "SELECT * FROM version");
    rc = sqlite3_exec(db, sql, _sql_call_get_lines, &versions, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :查询版本号失败: %s\n", __func__, err_msg);
        goto out;
    }

    line_count = 0;
    strcpy(sql, "SELECT * FROM configures");
    rc = sqlite3_exec(db, sql, _sql_call_get_lines, &configures, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :读取组件所有配置失败: %s\n", __func__, err_msg);
        goto out;
    }

    rc = sqlite3_exec(db, "COMMIT", NULL, NULL, &err_msg);
    if (rc != SQLITE_OK)
    {
        syslog(LOG_INFO, "[lingmosdk-conf2] %s ->  :无法提交事务: %s\n", __func__, err_msg);
        goto out;
    }
    sqlite3_close(db);

    if (!apps || !versions || !configures)
        return;

    // 顶层hash， 元素为appData:hash(versionData:hash(s:KSettingsSchema))
    if (NULL == schemas_table)
        schemas_table = g_hash_table_new_full(data_hash, data_equal, app_data_destroy, (GDestroyNotify)g_hash_table_destroy);
    if (NULL == schemas_table)
    {
        syslog(LOG_DEBUG, "[lingmosdk-conf2] %s ->  :Creat hash  Table failed\n", __func__);
        goto out;
    }

    int app_id = 0, version_id = 0;
    GHashTable *version = NULL;
    GHashTable *schemas = NULL;
    KSettingsSchema *schema = NULL;
    char curr_id[512] = {0};

#define APP_COLUMN apps[app_id - 1]
#define VER_COLUMN versions[version_id - 1]
    // 遍历配置
    for (int i = 0; configures[i]; i++)
    {
        // 版本id改变，说明当前版本的所有配置读取完成，开始读取下一个版本的配置
        if (version_id != atoi(configures[i][1]))
        {
            AppData *app_data = NULL;
            VersionData *version_data = NULL;
            syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Read a new version id %s, old id is %d\n", __func__, configures[i][1], version_id);
            version_id = atoi(configures[i][1]);

            version_data = calloc(1, sizeof *version_data);
            version_data->name = strdup(VER_COLUMN[2]);
            version_data->compatible = _split_string(VER_COLUMN[3], '.');

            // 版本id对应的应用id改变，说明当前app的所有配置读取结束，开始读取下一个app的配置。
            if (app_id != atoi(VER_COLUMN[1]))
            {
                syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Read a new app id %s, old id is %d\n", __func__, VER_COLUMN[1], app_id);
                app_id = atoi(VER_COLUMN[1]);

                app_data = calloc(1, sizeof *app_data);
                app_data->name = strdup(APP_COLUMN[1]);
                app_data->default_version = strdup(APP_COLUMN[2]);

                version = g_hash_table_lookup(schemas_table, app_data);
                if (NULL == version)
                {
                    version = g_hash_table_new_full(data_hash, data_equal, version_data_destroy, (GDestroyNotify)g_hash_table_destroy);
                    if (NULL == version)
                    {
                        syslog(LOG_DEBUG, "[lingmosdk-conf2] %s ->  :Creat hash2 Table failed\n", __func__);
                        app_data_destroy(app_data);
                        version_data_destroy(version_data);
                        goto out;
                    }
                    g_hash_table_insert(schemas_table, app_data, version);
                }
                else
                    app_data_destroy(app_data);
            }

            // 获取当前缓存的对应hash表，如果节点存在则更新hash表，不存在则创建一个
            schemas = g_hash_table_lookup(version, version_data);
            if (NULL == schemas)
            {
                schemas = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)schema_data_destroy);
                g_hash_table_insert(version, version_data, schemas);
                if (NULL == schemas)
                {
                    syslog(LOG_DEBUG, "[lingmosdk-conf2] %s ->  :Creat schemas hash Table failed\n", __func__);
                    version_data_destroy(version_data);
                    goto out;
                }
            }
            else
                version_data_destroy(version_data);
            syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Read %s configure wiht version number %s\n", __func__, APP_COLUMN[1], VER_COLUMN[2]);

            // 想节点中写入key以及key的所有数据
            schema = g_hash_table_lookup(schemas, APP_COLUMN[1]);
            if (NULL == schema)
            {
                syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Creat node %s\n", __func__, APP_COLUMN[1]);
                schema = calloc(1, sizeof *schema);
                if (NULL == schema)
                    goto out;

                schema->id = strdup(APP_COLUMN[1]);
                schema->version = strdup(VER_COLUMN[2]);
                g_hash_table_insert(schemas, strdup(APP_COLUMN[1]), schema);
            }
            else
            {
                syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Clear node %s\n", __func__, APP_COLUMN[1]);
                if (schema->values)
                {
                    g_hash_table_destroy(schema->values);
                    schema->values = NULL;
                }
                if (schema->children)
                {
                    g_array_free(schema->children, TRUE);
                    schema->children = NULL;
                }
            }
        }
        char *node_name = configures[i][2];
        char *node_type = configures[i][3];
        char *permission = configures[i][4];
        char *description = configures[i][5];
        char *summary = configures[i][6];
        char *value_type = configures[i][8];
        char *custom_value = configures[i][9];
        char *default_value = configures[i][10];
        char *value_range = configures[i][11];
        char *extends = configures[i][12];
        // 一个新的组件
        if (0 == strcmp(node_type, "schema"))
        {
            int parent_id = atoi(configures[i][7]);
            char parent_name[512], tmp[512];
            memset(parent_name, 0, 512);
            memset(tmp, 0, 512);
            if (0 == parent_id)
            {
                strcpy(parent_name, APP_COLUMN[1]);
            }
            else
            {
                while (0 != parent_id)
                {
                    strcat(tmp, ".");
                    strcat(tmp, configures[parent_id - 1][2]);
                    parent_id = atoi(configures[parent_id - 1][7]);
                }
                sprintf(parent_name, "%s%s", APP_COLUMN[1], tmp);
            }
            syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> %s's parent name %s\n", __func__, node_name, parent_name);

            // 给父组件增加子组件名
            KSettingsSchema *parent_schema = g_hash_table_lookup(schemas, parent_name);
            if (NULL != parent_schema)
            {
                if (NULL == parent_schema->children)
                {
                    parent_schema->children = g_array_new(FALSE, FALSE, sizeof(char *));
                }
                char *tmp = g_strdup(node_name);
                g_array_append_val(parent_schema->children, tmp);
            }

            char schema_name[512];
            memset(schema_name, 0, 512);
            sprintf(schema_name, "%s.%s", parent_name, node_name);
            schema = g_hash_table_lookup(schemas, schema_name);
            if (NULL == schema)
            {
                syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Creat node %s\n", __func__, schema_name);
                schema = calloc(1, sizeof *schema);
                if (NULL == schema)
                {
                    syslog(LOG_ERR, "[lingmosdk-conf2] %s -> Creat HashTable fail\n", __func__);
                    goto out;
                }

                schema->id = strdup(schema_name);
                schema->version = strdup(VER_COLUMN[2]);
                if (permission)
                    schema->permission = strdup(permission);
                if (description)
                    schema->description = strdup(description);
                if (summary)
                    schema->summary = strdup(summary);
                if (extends)
                    schema->extends = strdup(extends);
                g_hash_table_insert(schemas, strdup(schema_name), schema);
            }
            else
            {
                syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Clear node %s\n", __func__, APP_COLUMN[1]);
                if (schema->values)
                {
                    g_hash_table_destroy(schema->values);
                    schema->values = NULL;
                }
                if (schema->children)
                {
                    g_array_free(schema->children, TRUE);
                    schema->children = NULL;
                }
                g_free(schema->permission);
                g_free(schema->description);
                g_free(schema->summary);
                g_free(schema->extends);
                if (permission)
                    schema->permission = strdup(permission);
                if (description)
                    schema->description = strdup(description);
                if (summary)
                    schema->summary = strdup(summary);
                if (extends)
                    schema->extends = strdup(extends);
            }
        }
        else if (0 == strcmp(node_type, "key"))
        {
            syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> Read %s configure %s wiht version number %s\n", __func__, schema->id, node_name, schema->version);
            // hash表  s：hash（s:s）
            if (NULL == schema->values)
                schema->values = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_hash_table_destroy);
            // hash表 s:s
            GHashTable *item = g_hash_table_new(g_str_hash, g_str_equal);
            if (NULL == item)
            {
                syslog(LOG_ERR, "[lingmosdk-conf2] %s -> Memary error\n", __func__);
                goto out;
            }
            if (node_name)
                g_hash_table_insert(item, "node_name", g_strdup(node_name)); // 键名
            if (permission)
                g_hash_table_insert(item, "permission", g_strdup(permission)); // 读写权限
            if (description)
                g_hash_table_insert(item, "description", g_strdup(description)); // 详细描述
            if (summary)
                g_hash_table_insert(item, "summary", g_strdup(summary)); // 概要
            if (value_type)
                g_hash_table_insert(item, "value_type", g_strdup(value_type)); // 类型
            if (custom_value)
                g_hash_table_insert(item, "custom_value", g_strdup(custom_value)); // 键值
            if (default_value)
                g_hash_table_insert(item, "default_value", g_strdup(default_value)); // 默认值
            if (value_range)
                g_hash_table_insert(item, "range", g_strdup(value_range)); // 取值范围

            g_hash_table_insert(schema->values, g_strdup(node_name), item);
        }
    }
#undef APP_COLUMN
#undef VER_COLUMN

out:
    if (rc != SQLITE_OK)
    {
        sqlite3_exec(db, "ROLLBACK", 0, 0, &err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);

        g_hash_table_destroy(schemas_table);
        schemas_table = NULL;
    }
    if (apps)
    {
        for (int j = 0; apps[j]; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                if (apps[j][k])
                    free(apps[j][k]);
            }
            free(apps[j]);
        }
        free(apps);
    }
    if (versions)
    {
        for (int j = 0; versions[j]; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if (versions[j][k])
                    free(versions[j][k]);
            }
            free(versions[j]);
        }
        free(versions);
    }
    if (configures)
    {
        for (int j = 0; configures[j]; j++)
        {
            for (int k = 0; k < 13; k++)
            {
                if (configures[j][k])
                    free(configures[j][k]);
            }
            free(configures[j]);
        }
        free(configures);
    }
}

// 在schemas_table查找指定schema的配置句柄KSettingsSchema
KSettingsSchema *kdk_conf2_schema_table_lookup(const char *id, const char *version)
{
    KSettingsSchema *result = NULL;

    AppData *app_data = NULL;
    GHashTable *version_hash = NULL;
    char **list = _split_string(id, '.');

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, schemas_table);
    while (g_hash_table_iter_next(&iter, (gpointer*)&app_data, (gpointer*)&version_hash))
    {
        syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> find %s\n", __func__, app_data->name);
        if (0 == strcmp(app_data->name, list[0]))
        {
            syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> find app :%s\n", __func__, app_data->name);
            VersionData *version_data = NULL;
            GHashTable *schemas = NULL;
            version = version ? version : app_data->default_version;

            GHashTableIter iter_version;
            g_hash_table_iter_init(&iter_version, version_hash);
            while (g_hash_table_iter_next(&iter_version, (gpointer*)&version_data, (gpointer*)&schemas))
            {
                syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> find %s '%s configure\n", __func__, app_data->name, version_data->name);
                if (0 == strcmp(version_data->name, version))
                {
                    syslog(LOG_DEBUG, "[lingmosdk-conf2] %s -> find version :%s\n", __func__, version_data->name);
                    result = g_hash_table_lookup(schemas, id);
                    break;
                }
            }
        }
    }
    g_strfreev(list);
    return result;
}

void kdk_conf2_schema_destroy(KSettingsSchema *schema)
{
    schema_data_destroy(schema);
}

char *kdk_conf2_schema_get_id(KSettingsSchema *schema)
{
    return strdup(schema->id);
}

char *kdk_conf2_schema_get_version(KSettingsSchema *schema)
{
    return strdup(schema->version);
}

GHashTable *kdk_conf2_schema_get_key(KSettingsSchema *schema, const char *key)
{
    if (NULL == schema->values)
        return NULL;
    return (GHashTable *)g_hash_table_lookup(schema->values, key);
}

int kdk_conf2_schema_has_key(KSettingsSchema *schema)
{
    return schema->values ? TRUE : FALSE;
}

char **kdk_conf2_schema_list_keys(KSettingsSchema *schema)
{
    if (NULL == schema->values)
        return NULL;

    GList *keys = g_hash_table_get_keys(schema->values);
    guint len = g_list_length(keys);
    char **result = (char **)calloc(len + 1, sizeof(char *));
    if (NULL == result)
        return NULL;

    GList *iter = keys;
    for (guint i = 0; i < len; i++)
    {
        char *tmp = (char *)(iter->data);
        result[i] = (char *)calloc(strlen(tmp) + 1, sizeof(char));
        if (NULL == result[i])
        {
            // 无符号整形，-1为0xffffffff
            while (result[i] && i != 0xffffffff)
            {
                free(result[i--]);
            }
            free(result);
            break;
        }
        strcpy(result[i], tmp);
        iter = iter->next;
    }
    return result;
}

char **kdk_conf2_schema_list_children(KSettingsSchema *schema)
{
    if (NULL == schema->children)
        return NULL;

    char **result = (char **)calloc(schema->children->len + 1, sizeof(char *));
    if (NULL == result)
        return NULL;

    for (guint i = 0; i < schema->children->len; i++)
    {
        result[i] = (char *)calloc(strlen(g_array_index(schema->children, char *, i)) + 1, sizeof(char));
        if (NULL == result[i])
        {
            while (result[i] && i != 0xffffffff)
            {
                free(result[i--]);
            }
            free(result);
            break;
        }
        strcpy(result[i], g_array_index(schema->children, char *, i));
    }
    return result;
}

int kdk_conf2_schema_has_child(KSettingsSchema *schema, const char *name)
{
    if (NULL == schema->children)
        return FALSE;

    for (guint i = 0; i < schema->children->len; i++)
    {
        char *tmp = g_array_index(schema->children, char *, i);
        if (0 == g_strcmp0(name, tmp))
        {
            return TRUE;
        }
    }
    return FALSE;
}

#define GET_KEY_ATTR(x)                       \
    if (NULL == key)                          \
        return NULL;                          \
    char *tmp = g_hash_table_lookup(key, #x); \
    if (NULL == tmp)                          \
        return NULL;                          \
    char *result = strdup(tmp);               \
    return result;

char *kdk_conf2_schema_key_value_type(GHashTable *key)
{
    GET_KEY_ATTR(value_type)
}

char *kdk_conf2_schema_key_get_value(GHashTable *key)
{
    GET_KEY_ATTR(custom_value)
}

char *kdk_conf2_schema_key_get_default_value(GHashTable *key)
{
    GET_KEY_ATTR(default_value)
}

char *kdk_conf2_schema_key_get_range(GHashTable *key)
{
    GET_KEY_ATTR(range)
}

int kdk_conf2_schema_key_range_check(GHashTable *key, const char *value)
{
    int result = FALSE;
    char *type = kdk_conf2_schema_key_value_type(key);
    char *range = kdk_conf2_schema_key_get_range(key);

    // 无取值范围则数值合法
    if (NULL == range)
    {
        result = TRUE;
        goto out;
    }

    const gchar *endptr = NULL;
    GError *error = NULL;
    if (0 == strcmp(type, "enum"))
    {
        GVariant *input_value = g_variant_parse(G_VARIANT_TYPE("a{si}"), range, NULL, &endptr, &error);
        if (NULL == input_value)
        {
            g_print("解析失败: %s\n", error->message);
            g_error_free(error);
            goto out;
        }

        // GVariant *tmp = g_variant_parse(G_VARIANT_TYPE("i"), value, NULL, &endptr, &error);
        // if (NULL == tmp)
        // {
        //     g_error_free(error);
        //     error = NULL;
        // }

        char *nick = NULL;
        gint nick_value = 0;
        GVariantIter *iter = NULL;
        g_variant_get(input_value, "a{si}", &iter);
        while (g_variant_iter_loop(iter, "{si}", &nick, &nick_value))
        {

            // if (NULL == tmp)
            // {
            if (0 == g_strcmp0(value, nick))
                result = TRUE;
            // }
            // else
            // {
            //     if (atoi(value) == nick_value)
            //     {
            //         result = TRUE;
            //         g_variant_unref(tmp);
            //     }
            // }
        }
        g_variant_iter_free(iter);
        g_variant_unref(input_value);
    }
    else
    {
        GVariant *input_value = g_variant_parse(G_VARIANT_TYPE(type), value, NULL, &endptr, &error);
        if (NULL == input_value)
        {
            g_print("解析失败: %s\n", error->message);
            g_error_free(error);
            goto out;
        }
        char *p = range;
        while (*p != ',')
            p++;
        *p = '\0';
        GVariant *min_value = g_variant_parse(G_VARIANT_TYPE(type), range, NULL, &endptr, &error);
        if (NULL == min_value)
        {
            g_print("解析失败: %s\n", error->message);
            g_error_free(error);
            g_variant_unref(input_value);
            goto out;
        }
        GVariant *max_value = g_variant_parse(G_VARIANT_TYPE(type), p + 1, NULL, &endptr, &error);
        if (NULL == max_value)
        {
            g_print("解析失败: %s\n", error->message);
            g_error_free(error);
            g_variant_unref(input_value);
            g_variant_unref(min_value);
            goto out;
        }

        if ((g_variant_compare(min_value, input_value) <= 0) &&
            (g_variant_compare(max_value, input_value) >= 0))
            result = TRUE;
        g_variant_unref(input_value);
        g_variant_unref(min_value);
        g_variant_unref(max_value);
    }
out:
    g_free(type);
    g_free(range);
    return result;
}

char *kdk_conf2_schema_key_get_name(GHashTable *key)
{
    GET_KEY_ATTR(node_name)
}

char *kdk_conf2_schema_key_get_summary(GHashTable *key)
{
    GET_KEY_ATTR(summary)
}

char *kdk_conf2_schema_key_get_description(GHashTable *key)
{
    GET_KEY_ATTR(description)
}

char *kdk_conf2_schema_key_get_permission(GHashTable *key)
{
    GET_KEY_ATTR(permission);
}

void kdk_conf2_schema_update_schemas_table()
{
    //    if (schemas_table)
    //    {
    //        g_hash_table_destroy(schemas_table);
    //        schemas_table = NULL;
    //    }
    _get_schemas();
}

void kdk_conf2_schema_key_set_value(GHashTable *key, char *value)
{
    if (g_hash_table_contains(key, "custom_value"))
        g_hash_table_replace(key, "custom_value", strdup(value));
    else
        g_hash_table_insert(key, "custom_value", strdup(value));
}

void kdk_conf2_schema_key_reset(GHashTable *key)
{
    if (g_hash_table_contains(key, "custom_value"))
        g_hash_table_remove(key, "custom_value");
}

// 重新读取某个schema的配置
KSettingsSchema *kdk_conf2_schema_reload(const char *id, const char *version)
{
    KSettingsSchema *result = NULL;

    char **list = _split_string(id, '.');
    AppData *app_data = NULL;
    GHashTable *version_hash = NULL;

    GHashTableIter iter;
    g_hash_table_iter_init(&iter, schemas_table);
    while (g_hash_table_iter_next(&iter, (gpointer*)&app_data, (gpointer*)&version_hash))
    {
        if (0 == strcmp(app_data->name, list[0]))
        {
            VersionData *version_data = NULL;
            GHashTable *schemas = NULL;

            GHashTableIter iter_version;
            g_hash_table_iter_init(&iter_version, version_hash);
            while (g_hash_table_iter_next(&iter_version, (gpointer*)&version_data, (gpointer*)&schemas))
            {
                if (0 == strcmp(version_data->name, version))
                {
                    result = g_hash_table_lookup(schemas, id);
                    // 用新读取的配置替换掉schemas_table中的老配置
                    KSettingsSchema *schema = _get_schema(id, version);
                    if (NULL == schema)
                    {
                        syslog(LOG_ERR, "[lingmosdk-conf2] %s ->  :Update schema data failed\n", __func__);
                        break;
                    }

                    GHashTable *tmp = result->values;
                    result->values = schema->values;
                    schema->values = tmp;

                    GArray *tmp_array = result->children;
                    result->children = schema->children;
                    schema->children = tmp_array;
                    schema_data_destroy(schema);
                    break;
                }
            }
        }
    }
    g_strfreev(list);
    return result;
}
