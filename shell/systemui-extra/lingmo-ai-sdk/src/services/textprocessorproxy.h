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

#ifndef SERVICES_TEXTPROCESSORPROXY_H
#define SERVICES_TEXTPROCESSORPROXY_H

#include <functional>
#include <jsoncpp/json/value.h>
#include <string>
#include <lingmo-ai/ai-base/nlp.h>
#include "services/textprocessorglue.h"


class TextProcessorProxy {
public:
    using ChatCallback = std::function<void(NlpTextData result)>;

public:
    TextProcessorProxy();
    ~TextProcessorProxy();

    NlpResult initEngine(const std::string &engineName, const Json::Value &config);

    void setChatCallback(ChatCallback callback) {
        chatCallback_ = callback;
    }

    void setUserData(void *userData) { userData_ = userData; }

    NlpResult chat(const std::string &question);
    void stopChat();
    void setActor(int actor);
    void setContext(int size);
    void clearContext();

private:
    void init();
    void destroy();
    void connectSignal();
    void generateRandomSessionId();

    friend void onChatResult(GDBusConnection *connection,
                        const gchar      *sender_name,
                        const gchar      *object_path,
                        const gchar      *interface_name,
                        const gchar      *signal_name,
                        GVariant         *parameters,
                        gpointer          user_data);


private:
    AisdkTextProcessor *delegate_ = nullptr;

    ChatCallback chatCallback_ = nullptr;
    void *userData_ = nullptr;

    int sessionId_;
    int initErrorCode_; //0表示初始化成功，非0表示初始化失败
};
#endif
