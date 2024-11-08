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

#ifndef SERVICE_KEYWORDRECOGNIZERPROXY_H
#define SERVICE_KEYWORDRECOGNIZERPROXY_H

#include "services/keywordrecognizerglue.h"

#include <functional>

#include "speech.h"

class KeywordRecognizerProxy
{
public:
    using KeywordRecognitionResultCallback = std::function<void(RecognitionResult result)>;

    KeywordRecognizerProxy();
    ~KeywordRecognizerProxy();

    void setRecognizedCallback(KeywordRecognitionResultCallback callback);

    void initKeywordRecognizer();
    void destroyKeywordRecognizer();
    void startKeywordRecognizer();
    void stopKeywordRecognizer();

private:
    void init();
    void destroy();
    void connectSignals();
    friend RecognitionResult keyWordRecognizerResult(GVariant *parameters);
    friend void onKeywordRecognizerResult(GDBusConnection *connection,
                                          const gchar *sender_name,
                                          const gchar *object_path,
                                          const gchar *interface_name,
                                          const gchar *signal_name,
                                          GVariant *parameters,
                                          gpointer user_data);

private:
    AisdkKeywordRecognizer *delegate_ = nullptr;

    KeywordRecognitionResultCallback resultCallback_ = nullptr;
};

#endif