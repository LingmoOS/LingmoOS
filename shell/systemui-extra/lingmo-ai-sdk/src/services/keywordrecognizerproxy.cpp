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

#include "services/keywordrecognizerproxy.h"

#include <utility>
#include <iostream>

#include "services/serverproxy.h"

static const char *objectPath_ = "/org/lingmo/aisdk/keywordrecognizer";
static const char *interfaceName_ = "org.lingmo.aisdk.keywordrecognizer";

RecognitionResult keyWordRecognizerResult(GVariant *parameters)
{
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *char_text = g_variant_iter_next_value(&iter);
    GVariant *int_length = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    char *text = (char *)g_variant_get_data(char_text);
    int *length = (int *)g_variant_get_data(int_length);
    int *error = (int *)g_variant_get_data(int_error);

    RecognitionResult recognitionResult{
        text, (unsigned int)(*length), *error};
    return recognitionResult;
}

void onKeywordRecognizerResult(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data)
{
    std::cout << "func :" << __FUNCTION__ << std::endl;
    auto *proxy = reinterpret_cast<KeywordRecognizerProxy *>(user_data);
    if (proxy->resultCallback_ == nullptr)
    {
        std::cout << "KeywordRecognizer result callback is nullptr";
        return;
    }

    proxy->resultCallback_(keyWordRecognizerResult(parameters));
}

KeywordRecognizerProxy::KeywordRecognizerProxy() { init(); }

KeywordRecognizerProxy::~KeywordRecognizerProxy() { destroy(); }

void KeywordRecognizerProxy::init()
{
    const auto &unixPath = ServerProxy::getUnixPath();
    GError *error = nullptr;
    if (!ServerProxy::getInstance().available())
    {
        g_printerr("Error creating text processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }

    auto *connection = ServerProxy::getInstance().getConnection();

    delegate_ = aisdk_keyword_recognizer_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_,
        nullptr, &error);
    if (delegate_ == nullptr)
    {
        g_printerr("Error creating keyword recognizer proxy %s: %s\n",
                   objectPath_, error->message);
        g_error_free(error);
    }

    connectSignals();
}

void KeywordRecognizerProxy::destroy()
{
    if (delegate_ != nullptr)
    {
        g_object_unref(delegate_);
    }
}

void KeywordRecognizerProxy::connectSignals()
{
    GError *error = nullptr;
    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_keyword_recognizer_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_,
        nullptr, &error);
    if (delegate_ == nullptr)
    {
        g_printerr("Error creating keyword recognizer proxy %s: %s\n",
                   objectPath_, error->message);
        g_error_free(error);
    }

    g_dbus_connection_signal_subscribe(connection, nullptr,
                                       interfaceName_, "KeywordRecognizerResult", objectPath_, NULL,
                                       G_DBUS_SIGNAL_FLAGS_NONE, onKeywordRecognizerResult, this, nullptr);
}

void KeywordRecognizerProxy::setRecognizedCallback(KeywordRecognitionResultCallback callback)
{
    resultCallback_ = std::move(callback);
}

void KeywordRecognizerProxy::initKeywordRecognizer()
{
    GError *error = nullptr;
    aisdk_keyword_recognizer_call_init_keyword_recognizer_sync(delegate_, nullptr, &error);
}

void KeywordRecognizerProxy::destroyKeywordRecognizer()
{
    GError *error = nullptr;
    aisdk_keyword_recognizer_call_destroy_keyword_recognizer_sync(delegate_, nullptr, &error);
}

void KeywordRecognizerProxy::startKeywordRecognizer()
{
    GError *error = nullptr;
    aisdk_keyword_recognizer_call_start_keyword_recognizer_sync(delegate_, nullptr, &error);
}

void KeywordRecognizerProxy::stopKeywordRecognizer()
{
    GError *error = nullptr;
    aisdk_keyword_recognizer_call_stop_keyword_recognizer_sync(delegate_, nullptr, &error);
}