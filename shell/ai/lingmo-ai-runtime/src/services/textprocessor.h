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

#ifndef SERVICES_TEXTPROCESSOR_H
#define SERVICES_TEXTPROCESSOR_H

#include "services/textprocessorglue.h"
#include <string>
#include <memory>
#include <mutex>
#include <set>
#include <lingmo-ai/ai-engine/large-model/nlpengine.h>
#include "engine/aienginepluginmanager.h"
#include "config/prompttemplate.h"

class TextProcessor : public std::enable_shared_from_this<TextProcessor> {
public:
    TextProcessor(GDBusConnection &connection,
        ai_engine::AiEnginePluginManager &aiEnginePluginManager);

    ~TextProcessor();

    void stopProcess();

    static const std::string &getObjectPath() { return objectPath_; }

private:
    void exportSkeleton();
    void unexportSkeleton();
    int generateRandomSessionId();
    int getSdkErrorCode(int engineErrorCode);
    bool engineErrorCodeExists(int errorCode);

    bool initEngine(const std::string &engineName,
        const std::string &config, gint sessionId);
    int initSessionModule(gint sessionId);
    void setSessionChatCallback(gint sessionId);
    void sendChatResult(gint sessionId, const ai_engine::lm::nlp::ChatResult &result);
    void updateTokens(const std::string &chatResult);

    static bool onHandleInitEngine(AisdkTextProcessor *delegate,
        GDBusMethodInvocation *invocation, gchar *engineName, gchar *config,
        gpointer userData);

    static bool onHandleChat(
            AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
            gchar *question, gint sessionId, gpointer userData);

    static bool onSetActor(
            AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
            gint actor, gint sessionId, gpointer userData);

    static bool onStopChat(
            AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
            gint sessionId, gpointer userData);

    static bool onSetContextSize(
            AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
            gint size, gint sessionId, gpointer userData);

    static bool onClearContext(
            AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
            gint sessionId, gpointer userData);

private:
    std::shared_ptr<TextProcessor> getSharedObject() {
        return shared_from_this();
    }

private:
    bool isExported_ = false;
    AisdkTextProcessor *delegate_ = nullptr;
    GDBusConnection &connection_;
    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    std::unique_ptr<config::PromptTemplate> promptTemplate_;
    std::string currentPrompt_;
    int actorId_ { -1 };
    bool actorChanged { false };
    bool stopped { false };
    std::mutex mutex_;

    static const std::string objectPath_;

    static std::set<int> sessionIdSet;
    typedef std::map<gint, std::unique_ptr<ai_engine::lm::nlp::AbstractNlpEngine>> gintAndNlpEngineMap;
    gintAndNlpEngineMap nlpEngineMap_;

    bool useDefaultKey_{false};
};

#endif
