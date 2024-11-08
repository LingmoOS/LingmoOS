/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "engineconfiguration.h"
#include <fstream>
#include <jsoncpp/json/json.h>
#include <dbus/dbus.h>
#include <glib.h>

namespace config {

const char *promptTemplatesPath = "/etc/lingmo-ai/engines/ai-engine";

namespace utils {

Json::Value readJsonValue(const std::string &jsonFile) {
    std::ifstream ifs(jsonFile);
    if (!ifs.is_open()) {
        return Json::Value();
    }
    Json::Value root;
    ifs >> root;
    return root;
}

std::string readJsonMultiKeysStringValue(
    const Json::Value &value, std::initializer_list<std::string> keys) {
    Json::Value currentJsonObj = value;

    for (const auto &key : keys) {
        if (currentJsonObj.isMember(key)) {
            currentJsonObj = currentJsonObj[key];
        }
    }

    return currentJsonObj.isString() ? currentJsonObj.asString() : "";
}

char *getUser()
{
    char *e_user = NULL;
    char **users = NULL;

    DBusConnection *connection = NULL;
    DBusMessage *message = NULL, *reply = NULL;
    DBusMessageIter iter;
    int element_count = 0;

    // 初始化DBus连接
    DBusError error;
    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL) {
        fprintf(stderr, "DBus connection error: %s\n", error.message);
        goto out;
    }

    // 创建DBus方法调用消息
    message = dbus_message_new_method_call("org.freedesktop.login1",
                                           "/org/freedesktop/login1",
                                           "org.freedesktop.login1.Manager",
                                           "ListUsers");
    if (message == NULL) {
        fprintf(stderr, "DBus message allocation failed\n");
        goto out;
    }

    // 发送DBus消息并等待回复
    reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
    if (reply == NULL) {
        fprintf(stderr, "DBus reply error: %s\n", error.message);
        goto out;
    }

    // 释放message
    dbus_message_unref(message);
    message = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(reply, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
        dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_STRUCT) {
        fprintf(stderr, "DBus reply parsing failed\n");
        goto out;
    }

    // 释放reply
    dbus_message_unref(reply);
    reply = NULL;

    element_count = dbus_message_iter_get_element_count(&iter);
    users = (char**)calloc(element_count + 1, sizeof(char *));
    if (NULL == users) {
        goto out;
    }

    dbus_message_iter_recurse(&iter, &iter);

    // 解析返回值
    // 循环中过程中调用新的dbus接口会导致迭代器iter被清空，但是没细查原因
    // 先用一个字符串数组存储所有用户的object path
    for (int i = 0; i < element_count; i++) {
        char *path = NULL;

        DBusMessageIter struct_iter;
        dbus_message_iter_recurse(&iter, &struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_next(&struct_iter);
        dbus_message_iter_get_basic(&struct_iter, &path);

        users[i] = strdup(path);

        dbus_message_iter_next(&iter);
    }

    for (int i = 0; i < element_count; i++) {
        char *interface = "org.freedesktop.login1.User";
        char *state = "State";
        char *property = "Name";

        message = dbus_message_new_method_call("org.freedesktop.login1",
                                               users[i],
                                               "org.freedesktop.DBus.Properties",
                                               "Get");
        if (message == NULL) {
            fprintf(stderr, "DBus message allocation failed\n");
            goto out;
        }

        dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
        if (reply == NULL) {
            fprintf(stderr, "DBus reply error: %s\n", error.message);
            goto out;
        }

        dbus_message_unref(message);
        message = NULL;

        if (!dbus_message_iter_init(reply, &iter) ||
            dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
            fprintf(stderr, "DBus reply parsing failed\n");
            goto out;
        }

        dbus_message_unref(reply);
        reply = NULL;

        char *active = NULL;
        dbus_message_iter_recurse(&iter, &iter);
        dbus_message_iter_get_basic(&iter, &active);

        if (0 == strcmp(active, "active")) {
            message = dbus_message_new_method_call("org.freedesktop.login1",
                                                   users[i],
                                                   "org.freedesktop.DBus.Properties",
                                                   "Get");
            if (message == NULL) {
                fprintf(stderr, "DBus message allocation failed\n");
                goto out;
            }

            dbus_message_append_args(message, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID);

            reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);
            if (reply == NULL) {
                fprintf(stderr, "DBus reply error: %s\n", error.message);
                goto out;
            }

            dbus_message_unref(message);
            message = NULL;

            if (!dbus_message_iter_init(reply, &iter) ||
                dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
                fprintf(stderr, "DBus reply parsing failed\n");
                goto out;
            }

            dbus_message_unref(reply);
            reply = NULL;

            dbus_message_iter_recurse(&iter, &iter);
            dbus_message_iter_get_basic(&iter, &e_user);

            break;
        }
    }

    if (e_user) {
        e_user = strdup(e_user);
    }
out:
    if (users) {
        g_strfreev(users);
    }
    if (dbus_error_is_set(&error)) {
        dbus_error_free(&error);
    }
    if (message) {
        dbus_message_unref(message);
    }
    if (reply) {
        dbus_message_unref(reply);
    }
    if (connection) {
        dbus_connection_unref(connection);
    }
    return e_user;
}

char* getSystemLanguage(const char *username)
{
    if(!username) {
        return NULL;
    }

    char *user = NULL;
    DBusConnection *conn;
    DBusError err;

    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "DBus error: %s\n", err.message);
        dbus_error_free(&err);
    }

    if (NULL == conn) {
        return NULL;
    }

    DBusMessage *info_msg = NULL;
    DBusPendingCall *sendMsgPending = NULL;
    DBusMessage *replyMsg = NULL;

    //创建用户
    info_msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                            "/org/freedesktop/Accounts",     // object to call on
                                            "org.freedesktop.Accounts", // interface to call on
                                            "FindUserByName");           // method name
    if (!info_msg) { // -1 is default timeout
        fprintf(stderr, "DBus error: %s\n", "dbus_message_new_method_call调用失败\n");
        return NULL;
    }

    if (!dbus_message_append_args(info_msg, DBUS_TYPE_STRING, &username, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "DBus error: %s\n", "d-bus append args fail !\n");
        return NULL;
    }

    if (!dbus_connection_send_with_reply(conn, info_msg, &sendMsgPending, -1)) {
        fprintf(stderr, "DBus error: %s\n", "d-bus send message fail !\n");
        return NULL;
    }

    if (sendMsgPending == NULL) {
        fprintf(stderr, "DBus error: %s\n", "d-bus pending message is NULL !\n");
        return NULL;
    }

    dbus_connection_flush(conn);

    if (info_msg) {
        dbus_message_unref(info_msg);
    }

    dbus_pending_call_block(sendMsgPending);

    replyMsg = dbus_pending_call_steal_reply(sendMsgPending);

    if (replyMsg == NULL) {
        fprintf(stderr, "DBus error: %s\n", "get reply message fail !\n");
        return NULL;
    }

    if (sendMsgPending) {
        dbus_pending_call_unref(sendMsgPending);
    }

    if (dbus_message_get_type(replyMsg) == DBUS_MESSAGE_TYPE_ERROR) {
        fprintf(stderr, "DBus error: %s\n", dbus_message_get_error_name(replyMsg));
        dbus_message_unref(replyMsg);
        return NULL;
    }

    DBusMessageIter args;

    if (!dbus_message_iter_init(replyMsg, &args)) {
        dbus_message_unref(replyMsg);
        fprintf(stderr, "DBus error: %s\n", "d-bus reply message fail !\n");
        return NULL;
    } else {
        dbus_message_iter_get_basic(&args, &user);
    }

    if (replyMsg) {
        dbus_message_unref(replyMsg);
    }
    printf("user = %s\n", user);

    DBusMessage *msg = NULL;
    DBusPendingCall *sendPending = NULL;
    DBusMessage *replyLangMsg = NULL;

    //创建用户
    msg = dbus_message_new_method_call("org.freedesktop.Accounts",   // target for the method call
                                       user,     // object to call on
                                       "org.freedesktop.DBus.Properties", // interface to call on
                                       "Get");           // method name
    if (!msg) { // -1 is default timeout
        fprintf(stderr, "DBus error: %s\n", "dbus_message_new_method_call调用失败\n");
        return NULL;
    }
    char *interface = "org.freedesktop.Accounts.User";
    char *property = "Language";

    if (!dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface, DBUS_TYPE_STRING, &property, DBUS_TYPE_INVALID)) {
        fprintf(stderr, "DBus error: %s\n", "d-bus append args fail !\n");
        return NULL;
    }

    if (!dbus_connection_send_with_reply(conn, msg, &sendPending, -1)) {
        fprintf(stderr, "DBus error: %s\n", "d-bus send message fail !\n");
        return NULL;
    }

    if (sendPending == NULL) {
        fprintf(stderr, "DBus error: %s\n", "d-bus pending message is NULL !\n");
        return NULL;
    }

    dbus_connection_flush(conn);

    if (msg) {
        dbus_message_unref(msg);
    }

    dbus_pending_call_block(sendPending);

    replyLangMsg = dbus_pending_call_steal_reply(sendPending);

    if (replyLangMsg == NULL) {
        fprintf(stderr, "DBus error: %s\n", "get reply message fail !\n");
        return NULL;
    }

    if (sendPending) {
        dbus_pending_call_unref(sendPending);
    }

    if (dbus_message_get_type(replyLangMsg) == DBUS_MESSAGE_TYPE_ERROR) {
        fprintf(stderr, "DBus error: %s\n", dbus_message_get_error_name(replyLangMsg));
        dbus_message_unref(replyLangMsg);
        return NULL;
    }

    DBusMessageIter iter;
    char *lang = NULL;

    // 解析回复消息
    if (!dbus_message_iter_init(replyLangMsg, &iter) ||
        dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_VARIANT) {
        fprintf(stderr, "DBus error: %s\n", "DBus reply parsing failed\n");
        dbus_message_unref(replyLangMsg);
        dbus_connection_unref(conn);
        return NULL;
    }

    // 释放reply
    dbus_message_unref(replyLangMsg);

    dbus_message_iter_recurse(&iter, &iter);
    dbus_message_iter_get_basic(&iter, &lang);
    dbus_connection_unref(conn);
    return lang;
}

std::string getLanguageName() {
    char* eUser = getUser();
    char* language = getSystemLanguage(eUser);
    fprintf(stderr, "system_language = %s\n", language);
    if (std::strstr(language, "en")) {
        return "en_US";
    }
    if (std::strstr(language, "zh")) {
        return "zh_CN";
    }
    return "en_US";
}

}

EngineConfiguration::EngineConfiguration() {
    loadAiEngineConfig();
    loadDbEngineConfig();
}

std::string config::EngineConfiguration::currentLmSpeechEngineName() const
{
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, { "large-model", "current", "speech" });
}

std::string EngineConfiguration::currentLmNlpEngineName() const
{
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, { "large-model", "current", "nlp" });
}

std::string EngineConfiguration::currentLmVisionEngineName() const
{
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, { "large-model", "current", "vision" });
}

std::string EngineConfiguration::currentTmOcrEngineName() const
{
    return utils::readJsonMultiKeysStringValue(aiEngineConfig_, { "traditional-model", "current", "ocr" });
}

std::string EngineConfiguration::currentVdEngineName() const
{
    return utils::readJsonMultiKeysStringValue(dbEngineConfig_, { "traditional-model", "current", "ocr" });
}

std::string EngineConfiguration::promptTemplateConfigFile() const
{
    return promptTemplatesPath + "/" + currentLmNlpEngineName() + "/prompt.json";
}

std::string EngineConfiguration::promptFilePath() const
{
    return promptTemplatesPath + "/" + currentLmNlpEngineName() + "/prompts/" + utils::getLanguageName();
}

void EngineConfiguration::loadAiEngineConfig() {
    aiEngineConfig_ = utils::readJsonValue(aiEngineConfigFile_);
}

void EngineConfiguration::loadDbEngineConfig() {
    dbEngineConfig_ = utils::readJsonValue(dbEngineConfigFile_);
}

const std::string promptTemplateConfigFile(const std::string &engineName)
{
    return std::string(promptTemplatesPath) + "/" + engineName + "/prompt.json";
}

const std::string promptFilePath(const std::string &engineName)
{
    return std::string(promptTemplatesPath) + "/" + engineName + "/prompts/" + utils::getLanguageName();
}

} // namespace config
