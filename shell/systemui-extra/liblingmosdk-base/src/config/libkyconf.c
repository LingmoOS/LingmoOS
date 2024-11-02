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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


#include "libkyconf.h"
#include "structparse.h"
#include <kerr.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "jsonparse.h"
#include "xmlparse.h"
#include "gsettingsparse.h"

typedef enum
{
    KDK_CONF_XML,
    KDK_CONF_JSON,
    KDK_CONF_GSETTINGS,
    KDK_CONF_STRUCT
}KconfigureType;

typedef struct KconfigureSettings
{
    char *confpath;
    unsigned int id;
    int8_t watch;       // 监听配置变化
    KconfigureType type;
    void *data;         // 指向配置解析数据的指针
}KconfigureSettings;

typedef struct KconfigureList
{
    pthread_mutex_t lock;
    unsigned int counts;
    unsigned int max_nums;
    KconfigureSettings **conflist;
}KconfigureList;

static KconfigureList g_conflist;

static void init_conf_global()
{
    static int inited;
    // FixMe: 竞态风险
    if (inited)
        return;
    inited  = 1;
    pthread_mutex_init(&g_conflist.lock, NULL);
    g_conflist.counts = 0;
    g_conflist.max_nums = 5;
    g_conflist.conflist = NULL;
    return;
}

static KconfigureType get_conf_type(const char* confpath)
{
    if (isxml(confpath))
        return KDK_CONF_XML;
    else if (isjson(confpath))
        return KDK_CONF_JSON;
    else if (isgsettings(confpath))
        return KDK_CONF_GSETTINGS;
    return KDK_CONF_STRUCT;
}

static inline int kdk_test_file_exist(const char *file) ALWAYSINLINE;
static inline int kdk_test_file_exist(const char *file)
{
    return access(file, R_OK | F_OK) ? 0 : 1;
}

static inline int lock_conf_list() ALWAYSINLINE;
static inline int unlock_conf_list() ALWAYSINLINE;
static inline int lock_conf_list()
{
    return pthread_mutex_lock(&g_conflist.lock);
}

static inline int unlock_conf_list()
{
    return pthread_mutex_unlock(&g_conflist.lock);
}

static KconfigureSettings* new_conf_settings()
{
    KconfigureSettings *res = (KconfigureSettings *)calloc(1, sizeof(KconfigureSettings));
    ASSERT_NOT_NULL(res, NULL);
    lock_conf_list();
    if (! g_conflist.conflist)
    {
        g_conflist.conflist = (KconfigureSettings **)malloc(sizeof(KconfigureSettings *) * g_conflist.max_nums);
        if (! g_conflist.conflist)
            goto err;
    }
    if (g_conflist.counts == g_conflist.max_nums)
    {
        g_conflist.max_nums += 5;
        KconfigureSettings **tmp = g_conflist.conflist;
        g_conflist.conflist = (KconfigureSettings **)realloc(g_conflist.conflist, \
                                                            sizeof(KconfigureSettings *) * g_conflist.max_nums);
        if (! g_conflist.conflist)
        {
            g_conflist.conflist = tmp;
            g_conflist.max_nums -= 5;
            goto err;
        }
    }
    g_conflist.conflist[g_conflist.counts] = res;
    // counts仅单调递增
    g_conflist.counts ++;
    res->id = g_conflist.counts;
    unlock_conf_list();

    return res;
err:
    unlock_conf_list();
    free(res);
    return NULL;
}

static void free_conf_settings(KconfigureSettings *conf)
{
    if (conf)
    {
        lock_conf_list();
        // 根据配置文件的open/close频率特性，我们没有必要减少其引用计数和回收已经realloc的内存，置空就行了
        g_conflist.conflist[conf->id - 1] = NULL;
        unlock_conf_list();
        SAFE_FREE(conf->confpath);
        free(conf);
    }
}

int kdk_conf_init(const char* confpath)
{
    if (!confpath || !kdk_test_file_exist(confpath))
        return -KDK_EINVALIDARGS;

    init_conf_global();

    KconfigureSettings *conf = new_conf_settings();
    ASSERT_NOT_NULL(conf, -1);

    conf->confpath = (char *)malloc(sizeof(char) * (strlen(confpath) + 1));
    if (! conf->confpath)
        goto err;
    
    strcpy(conf->confpath, confpath);

    conf->type  = get_conf_type(confpath);

    if (conf->type == KDK_CONF_XML)
    {

    }
    else if (conf->type == KDK_CONF_JSON)
    {

    }
    else if (conf->type == KDK_CONF_GSETTINGS)
    {

    }
    else
    {
        conf->data = S_newParse();
        if (!conf->data)
            goto err;

        if (S_parseFile(conf->data, conf->confpath))
        {
            goto err;
        }
    }

    return conf->id;

err:
    free_conf_settings(conf);
    return -1;
}

void kdk_conf_destroy(int id)
{
    if (id > 0)
        free_conf_settings(g_conflist.conflist[id - 1]);
}

int kdk_conf_reload(int id)
{
    if (id <= 0)
        return -KDK_EINVALIDARGS;
    
    KconfigureSettings *conf = g_conflist.conflist[id - 1];
    ASSERT_NOT_NULL(conf, -1);

    if (conf->type == KDK_CONF_XML)
    {

    }
    else if (conf->type == KDK_CONF_JSON)
    {

    }
    else if (conf->type == KDK_CONF_GSETTINGS)
    {

    }
    else
    {
        void *data = S_newParse();
        if (! data)
            goto err;

        if (S_parseFile(data, conf->confpath))
        {
            free(data);
            goto err;
        }

        S_destroyParse((structParse**)(&conf->data));
        conf->data = data;
    }

    return 0;
err:
    return -1;
}

void kdk_conf_enable_autoreload(int id)
{
#ifdef LINGMOSDK_FILEWATCHER_H
#endif // LINGMOSDK_FILEWATCHER_H
    return;
}

void kdk_conf_disable_autoreload(int id)
{
#ifdef LINGMOSDK_FILEWATCHER_H
#endif // LINGMOSDK_FILEWATCHER_H
    return;
}

const char* kdk_conf_get_value(int id, const char* group, const char* key)
{
    if (id <= 0 || id > g_conflist.counts || ! key)
        return NULL;
    
    if (!group || strlen(group) == 0)
        group = "KDK_DefaultGroup";
    const char *res = "";

    KconfigureSettings *conf = g_conflist.conflist[id - 1];
    ASSERT_NOT_NULL(conf, NULL);

    if (conf->type == KDK_CONF_XML)
    {

    }
    else if (conf->type == KDK_CONF_JSON)
    {

    }
    else if (conf->type == KDK_CONF_GSETTINGS)
    {

    }
    else
    {
        res = S_getValue(conf->data, group, key);
    }

    return res;
}

int kdk_conf_set_value(int id, const char* group, const char* key, const char* value)
{
    return 0;
}

char** const kdk_conf_list_key(int id, const char* group)
{
    if (id < 1 || id > g_conflist.counts || ! group)
        return NULL;

    char** res = NULL;
    KconfigureSettings *conf = g_conflist.conflist[id - 1];
    ASSERT_NOT_NULL(conf, NULL);

    if (conf->type == KDK_CONF_XML)
    {

    }
    else if (conf->type == KDK_CONF_JSON)
    {

    }
    else if (conf->type == KDK_CONF_GSETTINGS)
    {

    }
    else
    {
        res = S_getKeyList(conf->data, group);
    }

    return res;
}

char** const kdk_conf_list_group(int id)
{
    if (id < 1)
        return NULL;
    
    char** res = NULL;
    KconfigureSettings *conf = g_conflist.conflist[id - 1];
    ASSERT_NOT_NULL(conf, NULL);

    if (conf->type == KDK_CONF_XML)
    {

    }
    else if (conf->type == KDK_CONF_JSON)
    {

    }
    else if (conf->type == KDK_CONF_GSETTINGS)
    {

    }
    else
    {
        res = S_getGroupList(conf->data);
    }

    return res;
}

inline void kdk_config_freeall(char **list)
{
    if (! list)
        return;
    size_t index = 0;
    while (list[index])
    {
        free(list[index]);
        index ++;
    }
    free(list);
}
