#include "libkyglobal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lingmosdk/lingmosdk-base/cstring-extension.h>
#include <unistd.h>
#include <libkylocation.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <dbus-1.0/dbus/dbus.h>
#include <libkylog.h>


#define KDK_LANGUAGE_FILE                   "/usr/share/i18n/SUPPORTED"
#define KDK_SYSTEM_LANGUAGE                 "LANG"

char **kdk_global_get_system_support_language()
{
    FILE *fp = NULL;
    char line[128] = "\0";

    fp = fopen(KDK_LANGUAGE_FILE, "rt");
    if (!fp)
        return NULL;

    char **language = NULL;
    char **tmp = NULL;
    int index = 0;
    while(fgets(line, sizeof(line), fp))
    {
        tmp = (char **)realloc(language, sizeof(char *) * (index + 2));
        if(!tmp)
        {
            goto err_out;
        }
        language = tmp;
        language[index] = (char *)malloc(sizeof(line));
        if(!language[index])
        {
            goto err_out;
        }
        strstripspace(line);
        strcpy(language[index], line);
        index++;
    }
    fclose(fp);
    language[index] = NULL;
    return language;

err_out:
    while (index)
    {
        free(language[index - 1]);
        index--;
    }
    free(language);
    fclose(fp);
    return NULL;
}

bool kdk_global_get_region_match_language()
{
    char area[12] = "\0";
    char * lang = getenv(KDK_SYSTEM_LANGUAGE);
    sscanf(lang, "%*[^_]_%2s", area);

    char *location = kdk_loaction_get();
    cJSON *json = cJSON_Parse(location);
    if (json == NULL) {
        return false;
    }
    // 获取 outputs 数组
    cJSON *outputs = cJSON_GetObjectItem(json, "address");
    if (strlen(outputs->valuestring) != 0)
    {
        char **fg = strsplit(outputs->valuestring, '|');
        if (!fg)
        {
            return false;
        }

        if(strcmp(area, fg[0]) == 0)
        {
            free(fg);
            return true;
        }
        else if(strcmp(area, "TW") == 0 && strcmp(fg[0], "CN") == 0)
        {
            free(fg);
            return true;
        }
        else if(strcmp(area, "HK") == 0 && strcmp(fg[0], "CN") == 0)
        {
            free(fg);
            return true;
        }
        else if(strcmp(area, "MO") == 0 && strcmp(fg[0], "CN") == 0)
        {
            free(fg);
            return true;
        }
        else
        {
            free(fg);
            return false;
        }
    }
    return false;
}

bool kdk_global_get_rtl()
{
    char *language[] = {"ar_AE", "ar_BH", "ar_DZ", "ar_EG", "ar_IN", "ar_IQ", "ar_JO", "ar_KW", "ar_LB", "ar_LY", "ar_MA", "ar_OM", "ar_QA", 
                        "ar_SA", "ar_SD", "ar_SS", "ar_SY", "ar_TN", "ar_YE", "fa_IR", "ug_CN", "he_IL", "ur_IN", "ur_PK", "kk_KZ", "ky_KG"};

    char *lang = getenv(KDK_SYSTEM_LANGUAGE);
    int size = (sizeof(language) / sizeof(language[0]));
    // int i = 0;
    for(int i = 0; i < size; i++)
    {
        if(strstr(lang, language[i]))
        {
            return true;
        }
    }
    return false;
}

int kdk_global_get_raw_offset()
{
    time_t t;
    struct tm *local_tm;
    char buffer[10] = "\0";  // 用于存储格式化后的时区字符串
    char buff[10] = "\0";

    // 获取当前时间
    time(&t);

    // 获取本地时间
    local_tm = localtime(&t);

    // 使用 strftime 格式化时区偏移量，格式为 ±hh:mm
    strftime(buffer, sizeof(buffer), "%z", local_tm);
    sprintf(buff, "%.3s", buffer);
    int offset = atoi(buff);
    return offset;
}

char* kdk_global_get_system_language(const char *username)
{
    if(!username)
        return NULL;

    char *user = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err))
    {
        klog_err("Connection Error (%s)\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn)
    {
        goto err_out;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            "/org/freedesktop/Accounts",     // object to call on
                                            "org.freedesktop.Accounts", // interface to call on
                                            "FindUserByName");           // method name
    if (!info_msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &username, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendMsgPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
    }

    dbus_connection_flush(conn);

    if (info_msg)
    {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);

    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

    if (replyMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendMsgPending)
    {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return NULL;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args))
    {
        dbus_message_unref(replyMsg);
        klog_err("kdk : d-bus reply message fail !\n");
        goto err_out;
    }
    else
    {
        dbus_message_iter_get_basic(&args, &user);
    }

    if (replyMsg)
    {
        dbus_message_unref(replyMsg);
    }

    DBusMessage *msg = NULL;
    DBusPendingCall *sendPending = NULL;
    DBusMessage *replyLangMsg = NULL;

    //创建用户
    msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            user,     // object to call on
                                            "org.freedesktop.DBus.Properties", // interface to call on
                                            "Get");           // method name
    if (!msg)
    { // -1 is default timeout
        klog_err("status_msg: dbus_message_new_method_call调用失败\n");
        goto err_out;
    }
    char *interface = "org.freedesktop.Accounts.User";
    char *property = "Language";

    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID))
    {
        klog_err("kdk : d-bus append args fail !\n");
        goto err_out;
    }

    if (!dbus_connection_send_with_reply(conn, msg, &sendPending, -1))
    {
        klog_err("kdk : d-bus send message fail !\n");
        goto err_out;
    }

    if (sendPending == NULL)
    {
        klog_err("kdk : d-bus pending message is NULL !\n");
        goto err_out;
    }

    dbus_connection_flush(conn);

    if (msg)
    {
        dbus_message_unref(msg);
    }

    dbus_pending_call_block(sendPending);

    replyLangMsg = dbus_pending_call_steal_reply(sendPending);

    if (replyLangMsg == NULL)
    {
        klog_err("kdk : get reply message fail !\n");
        goto err_out;
    }

    if (sendPending)
    {
        dbus_pending_call_unref(sendPending);
    }

    if (dbus_message_get_type(replyLangMsg) == DBUS_MESSAGE_TYPE_ERROR) 
    {
        klog_err("DBus error: %s\n", dbus_message_get_error_name(replyLangMsg));
        dbus_message_unref(replyLangMsg);
        goto err_out;
    }

    DBusMessageIter iter;
    char *lang = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(replyLangMsg, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT)
    {
        klog_err("DBus reply parsing failed\n");
        dbus_message_unref(replyLangMsg);
        dbus_connection_unref(conn);
        goto err_out;
    }

    // 释放reply
    dbus_message_unref(replyLangMsg);

    dbus_message_iter_recurse(&iter, &iter);
    dbus_message_iter_get_basic(&iter, &lang);
    dbus_connection_unref(conn);
    char *language = NULL;
    if(lang)
    {
        language = (char *)malloc(sizeof(lang) + 1);
        if(!language)
            goto err_out;
        strcpy(language, lang);
    }
    
    return language;

err_out:
    return NULL;
}


void kdk_free_langlist(char** langlist)
{
    if (! langlist)
        return;
    size_t index = 0;
    while (langlist[index])
    {
        free(langlist[index]);
        index ++;
    }
    free(langlist);
}