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

#include "services/textprocessorproxy.h"
#include "nlp.h"
#include "services/serverproxy.h"
#include <iostream>

namespace {

const char *textProcessorObjectPath = "/org/lingmo/aisdk/textprocessor";
const char *textProcessorInterfaceName = "org.lingmo.aisdk.textprocessor";

}

void onChatResult(GDBusConnection *connection,
                  const gchar      *sender_name,
                  const gchar      *object_path,
                  const gchar      *interface_name,
                  const gchar      *signal_name,
                  GVariant         *parameters,
                  gpointer          user_data) {
    auto *proxy = static_cast<TextProcessorProxy *>(user_data);

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *char_result = g_variant_iter_next_value(&iter);
    GVariant *int_errorCode = g_variant_iter_next_value(&iter);

    if (!char_result || !int_errorCode) {
        fprintf(stderr, "set chat result callback error: result is nullptr!\n");
        return;
    }

    char *result = (char*)g_variant_get_data(char_result);
    int *errorCode = (int *)g_variant_get_data(int_errorCode);
    if (!result || !errorCode) {
        fprintf(stderr, "set chat result callback error: error code is nullptr!\n");
        return;
    }

    NlpTextData nlpTextData { (unsigned char*)result, *errorCode };
    proxy->chatCallback_(nlpTextData);
}

TextProcessorProxy::TextProcessorProxy() { init(); }

TextProcessorProxy::~TextProcessorProxy() { destroy(); }

NlpResult TextProcessorProxy::initEngine(const std::string &engineName,
    const Json::Value &config)
{
    GError *error = nullptr;
    aisdk_text_processor_call_init_sync(
                delegate_, engineName.c_str(),
                config.toStyledString().c_str(), &sessionId_,
                &initErrorCode_, nullptr, &error);

    if (error) {
        fprintf(stderr, "Init engine error: %s\n", error->message);
        return NlpResult::NLP_SERVER_ERROR;
    }

    fprintf(stdout, "proxy init engine sessionId %i\n", sessionId_);
    connectSignal();
    return (NlpResult)initErrorCode_;
}

NlpResult TextProcessorProxy::chat(const std::string &question) {
    if (initErrorCode_) {
        return (NlpResult)initErrorCode_;
    }
    aisdk_text_processor_call_chat(
        delegate_, question.c_str(), sessionId_, nullptr, nullptr, this);
    return NlpResult::NLP_SUCCESS;
}

void TextProcessorProxy::stopChat()
{
    aisdk_text_processor_call_stop_chat(delegate_, sessionId_,
                                        nullptr, nullptr, nullptr);
}

void TextProcessorProxy::setActor(int actor)
{
    aisdk_text_processor_call_set_actor(
        delegate_, actor, sessionId_, nullptr, nullptr, nullptr);
}

void TextProcessorProxy::setContext(int size)
{
    aisdk_text_processor_call_set_context(delegate_, size, sessionId_,
                                          nullptr, nullptr, nullptr);
}

void TextProcessorProxy::clearContext()
{
    aisdk_text_processor_call_clear_context(delegate_, sessionId_,
                                            nullptr, nullptr, nullptr);
}

void TextProcessorProxy::init() {
    const auto &unixPath = ServerProxy::getUnixPath();
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating text processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_text_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, textProcessorObjectPath,
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating text processor proxy %s: %s\n",
                   textProcessorObjectPath, error->message);
        g_error_free(error);
    }
}

void TextProcessorProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

void TextProcessorProxy::connectSignal() {
    auto *connection = ServerProxy::getInstance().getConnection();
    std::string interfaceName = textProcessorInterfaceName + std::to_string(sessionId_);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName.c_str(), "ChatResult", textProcessorObjectPath, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onChatResult, this, nullptr);
}
