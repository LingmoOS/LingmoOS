#include "sync-dbus-server.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <gio/gsettings.h>
#include <libkylog.h>
#include <../api/libkysettings.h>

/***************************************dbus服务********************************************/

static SyncConfig *skeleton = NULL;

static gboolean on_handle_register_schema(SyncConfig *skeleton, GDBusMethodInvocation *invocation,
                                          const gchar *arg_Conf2Id,
                                          const gchar *arg_SchemaId,
                                          const gchar *arg_Path,
                                          gpointer user_data)
{
    printf("Regester call %s\n", arg_Conf2Id);

    sync_config__complete_register_schema(skeleton, invocation, TRUE);
    return TRUE;
}

static gboolean on_handle_unregister_schema(SyncConfig *skeleton, GDBusMethodInvocation *invocation,
                                            const gchar *arg_SchemaId,
                                            const gchar *arg_Path,
                                            gpointer user_data)
{
    printf("Unregester call\n");
    sync_config__complete_unregister_schema(skeleton, invocation, TRUE);
    return TRUE;
}

/**
 * 连接上bus daemon的回调
 **/
void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
    klog_info("on_bus_acquired has been invoked, name is %s\n", name);
    GError *error = NULL;
    skeleton = sync_config__skeleton_new();
    // 连接方法同步处理函数
    g_signal_connect(G_OBJECT(skeleton), "handle-register-schema", G_CALLBACK(on_handle_register_schema), NULL);
    g_signal_connect(G_OBJECT(skeleton), "handle-unregister-schema", G_CALLBACK(on_handle_unregister_schema), NULL);
    // 发布服务到总线
    g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(skeleton), connection, "/com/lingmo/lingmosdk/syncConfig", &error);
    if (error != NULL)
    {
        klog_err("Error: Failed to export object. Reason: %s.\n", error->message);
        g_error_free(error);
    }
}

/**
 * 成功注册busName的回调
 **/
void on_bus_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

/**
 * busName丢失的回调，一般是server挂了？？
 **/
void on_bus_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
}

/**
 * 测试发送信号函数，调用Hello.h自动生成的方法
 **/
static gint status_value = 1;
static gboolean emit_test_state_signal(gconstpointer p)
{
    printf("emit_test_status_signal invoked\n");
    if (skeleton != NULL)
    {
        sync_config__emit_state(skeleton, status_value);
    }
    status_value++;
}

gboolean init_gdbus_server(GBusType type)
{
    guint owner_id;
    GMainLoop *loop;
#if !GLIB_CHECK_VERSION(2, 35, 0)
    g_type_init();
#endif
    owner_id = g_bus_own_name(type, "com.lingmo.lingmosdk.SyncConfig",
                              G_BUS_NAME_OWNER_FLAGS_NONE,
                              on_bus_acquired, on_bus_name_acquired, on_bus_name_lost, NULL, NULL);

    // 测试，每3s发一次signal
    // g_timeout_add(3000, (GSourceFunc)emit_test_state_signal, NULL);

    // 主线程进入循环
    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_bus_unown_name(owner_id);

    return TRUE;
}

/***************************************同步配置********************************************/
#define OUT
#define KPATH_LEN 512
#define APP_LEN 128
#define VER_LEN 32
#define LINE_LEN 4096

struct _conf2_gsetting_handle_duple
{
    KSettings *ksetting;
    GSettings *gsetting;
};
// static GPtrArray *handle_array = g_ptr_array_new();


static void _on_gsettings_key_changed(GSettings *gsetting, const char *key, void *user_data)
{
    GVariant *value = g_settings_get_value(gsetting, key);
    KSettings *ksetting = (KSettings *)user_data;
    char *conf2_value = kdk_conf2_get_value(ksetting, key);
    if (0 == strcmp(conf2_value, g_variant_print(value, TRUE)))
        return;
    int ret = kdk_conf2_set_value(ksetting, key, g_variant_print(value, TRUE));
    if (ret)
    {
        printf("_on_gsettings_key_changed\n");
        // g_signal_emit(ksetting, k_settings_signals[SIGNAL_CHANGED], g_quark_from_string(key), key);
    }
    g_variant_unref(value);
}

static void _on_conf2_key_changed(KSettings *ksetting, const char *key, void *user_data)
{
    GSettings *gsetting = (GSettings*)user_data;
    const gchar *endptr = NULL;
    GError *error = NULL;
    GVariant *gsetting_value = g_settings_get_value(gsetting, key);
    if (NULL != gsetting_value)
    {
        char *value = kdk_conf2_get_value(ksetting, key);
        if (0 != strcmp(value, g_variant_print(gsetting_value, TRUE)))
        {
            char *type = kdk_conf2_get_type(ksetting, key);
            GVariant *variant = g_variant_parse(G_VARIANT_TYPE(type), value, NULL, &endptr, &error);
            if (NULL == variant)
            {
                // klog_err("[lingmosdk-conf2] %s -> %s\n", __func__, error->message);
                return;
            }
            printf("_on_conf2_key_changed\n");
            g_settings_set_value(gsetting, key, variant);
            g_settings_sync();
            g_variant_unref(variant);
        }
        g_variant_unref(gsetting_value);
    }
}

static char *_conf2_id_in_gsetting_schemas(const char *conf2_id, const char **gsetting_schemas)
{
    for(; *gsetting_schemas; gsetting_schemas++)
    {
        if (g_str_has_suffix(*gsetting_schemas, conf2_id))
            return *gsetting_schemas;
    }
    return NULL;
}

/**
 * @brief 比较两个版本号的大小
 * 
 * @param version1 版本号
 * @param version2 版本号
 * @return int version1大于version时返回1， 等于返回0， 小于返回-1
 */
static int _compare_versions(char *version1, char *version2)
{
    if (version1 != NULL && version2 == NULL)
        return 1;
    else if (version1 == NULL && version2 != NULL)
        return -1;
    else if (version1 == NULL && version2 == NULL)
        return 0;

    int result = 0;

    char v1[64], v2[64];
    memset(v1, 0, 64);
    strcpy(v1, version1);
    memset(v2, 0, 64);
    strcpy(v2, version2);

    char *save_ptr1 = NULL;
    char *save_ptr2 = NULL;
    char *token1;
    char *token2;
    token1 = strtok_r(v1, ".-k", &save_ptr1);
    token2 = strtok_r(v2, ".-k", &save_ptr2);

    while (1)
    {
        if (token1 != NULL && token2 != NULL)
        {
            int num1 = atoi(token1);
            int num2 = atoi(token2);

            if (num1 > num2)
            {
                result = 1;
                break;
            }
            else if (num1 < num2)
            {
                result = -1;
                break;
            }
        }
        else if (token1 == NULL && token2 != NULL)
        {
            result = -1;
            break;
        }
        else if (token1 != NULL && token2 == NULL)
        {
            result = 1;
            break;
        }
        else if (token1 == NULL && token2 == NULL)
            break;

        token1 = strtok_r(NULL, ".-k", &save_ptr1);
        token2 = strtok_r(NULL, ".-k", &save_ptr2);
    }

    return result;
}

static inline gboolean _copy_str_check(char *dest, char *src, size_t length)
{
    // 去掉前面空格
    g_strchug(src);
    /*本行内容应该是 【应用名:】，如果不是以":"结尾，说明文件内容格式错误*/
    char *dot = strchr(src, ':');
    if (NULL != dot)
    {
        int len = dot - src;
        strncpy(dest, src, len < length - 1 ? len : length);
        return TRUE;
    }
    return FALSE;
}

static gboolean _get_conf2_app(FILE *fp, OUT char *app)
{
    gboolean result = FALSE;

    char line[LINE_LEN], *dot = NULL;
    int version_indent = 0;
    while (fgets(line, LINE_LEN, fp))
    {
        // 无缩进且不是注释，既第一行内容
        dot = strchr("#\t ", line[0]);
        if (NULL == dot)
        {
            result = _copy_str_check(app, line, APP_LEN);
            break;
        }
    }
    return result;
}

static gboolean _get_conf2_version(FILE *fp, OUT char *version)
{
    gboolean result = FALSE;

    char line[LINE_LEN], *dot = NULL;
    int version_indent = 0;
    while (fgets(line, LINE_LEN, fp))
    {
        // 获取版本号的缩进,缩进最短的正文既版本号
        if (0 == version_indent)
        {
            for (;; version_indent++)
            {
                // line[version_indent]不是空格
                if (NULL == strchr("\t ", line[version_indent]))
                {
                    // 注释行，缩进计数归零
                    if ('#' == line[version_indent])
                        version_indent = 0;
                    else
                    {
                        // 这行正文既一个版本号
                        if (FALSE == _copy_str_check(version, line, VER_LEN))
                        {
                            printf("111%s\n", line);
                            return result;
                        }
                    }
                    break; // for循环
                }
            }
            continue; // while循环
        }
        // 检查这一行是不是版本号
        int i = 0;
        for (; i < version_indent; i++)
        {
            // 空格不够
            if (NULL == strchr("\t ", line[i]))
            {
                // 注释行或空行
                if (NULL != strchr("#\n", line[i]))
                    break; // for循环
                else
                {
                    // TODO: log 文件缩进格式错误
                    printf("222%s\n", line);
                    return result;
                }
            }
        }
        if (i == version_indent)
        {
            if((line[i] != '#') && (line[i] != ' ') && (line[i] != '\t'))
            {

                char tmp[VER_LEN];
                memset(tmp, 0, VER_LEN);
                if (FALSE == _copy_str_check(tmp, line, VER_LEN))
                {
                    // TODO: log 文件格式错误
                    printf("333%s\n", line);
                    return result;
                }
                if ( -1 == _compare_versions(version, tmp))
                {
                    strcpy(version, tmp);
                }
            }
        }
    }
    result = TRUE;
    return result;
}

/*
自解析yaml格式文件，并获取应用名和版本号。
注意:dbus-daemon会把自己的文件安装到basic目录，且后缀名为yaml，但
文件内部的格式并不是yaml格式
 */
static gboolean _get_conf2_app_and_version(const char *file, OUT char *app, OUT char *version)
{
    gboolean result = FALSE;

    memset(app, 0, APP_LEN);
    memset(version, 0, VER_LEN);

    FILE *fp = fopen(file, "r");
    if (NULL == fp)
    {
        // TODO: 文件打开失败
        return result;
    }

    result = _get_conf2_app(fp, app);
    if (FALSE == result)
    {
        // TODO: log 文件内容不符合yaml格式
        goto out;
    }
    result = _get_conf2_version(fp, version);
    if (FALSE == result)
    {
        // TODO: log 文件内容不符合yaml格式
        goto out;
    }
out:
    fclose(fp);
    return result;
}

static gboolean _connect_signal_sync_config(const char *conf2_id, const char *conf2_version, const char *gsetting_id, const char *gsetting_path)
{
    GSettings *gsetting = NULL;
    KSettings *ksetting = NULL;
    if ( NULL == gsetting_path)
        gsetting = g_settings_new(gsetting_id);
    else
        gsetting = g_settings_new_with_path(gsetting_id, gsetting_path);

    ksetting = kdk_conf2_new(conf2_id, conf2_version);

    if (!(gsetting && ksetting))
    {
        // TODO: log 创建句柄失败
        g_object_unref(gsetting);
        kdk_conf2_ksettings_destroy(ksetting);
        return FALSE;
    }
    g_signal_connect(gsetting, "changed", G_CALLBACK(_on_gsettings_key_changed), ksetting);
    kdk_conf2_connect_signal(ksetting, "changed", (KCallBack)_on_conf2_key_changed, gsetting);
    return TRUE;
}

int monitoring_requires_synchronized_configuration()
{
    const char * const *gsetting_schemas = g_settings_list_schemas();

    DIR *dir = NULL;
    struct dirent *dire = NULL;
    dir = opendir("/etc/lingmo-config/basic");
    if (NULL == dir)
    {
        return -1;
    }

    while (NULL != (dire = readdir(dir)))
    {
        char file_name[KPATH_LEN];
        memset(file_name, 0, KPATH_LEN);
        char *dot = strrchr(dire->d_name, '.');
        if (NULL == dot)
        {
            // log
            continue;
        }
        size_t len = dot - dire->d_name;
        strncpy(file_name, dire->d_name, len);
        // 文件名与gsetting的id相同，需要同步
        if (g_strv_contains(gsetting_schemas, file_name))
        {
            // 获取应用名和版本号
            g_snprintf(file_name, KPATH_LEN, "/etc/lingmo-config/basic/%s", dire->d_name);
            char app[APP_LEN], version[VER_LEN];
            memset(app, 0, APP_LEN);
            memset(version, 0, VER_LEN);
            if (FALSE == _get_conf2_app_and_version(file_name, app, version))
            {
                // TODO: 记录日志读取文件内应用与app失败
                continue;
            }
            char **ksetting_schemas = kdk_conf2_list_schemas(app, version);
            if (NULL == ksetting_schemas)
            {
                //TODO: log 获取conf2中app的version版本的id列表失败
                continue;
            }
            for(char **tmp = ksetting_schemas; *tmp; tmp++)
            {
                char *gsetting_id = NULL;
                if(NULL != (gsetting_id = _conf2_id_in_gsetting_schemas(*tmp, gsetting_schemas)))
                    _connect_signal_sync_config(*tmp, version, gsetting_id, NULL);
            }
            g_strfreev(ksetting_schemas);
        }
    }
    closedir(dir);
}