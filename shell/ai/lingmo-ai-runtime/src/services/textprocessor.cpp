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

#include "services/textprocessor.h"
#include <cstdio>
#include <lingmo-ai/ai-engine/large-model/cloudnlpengine.h>
#include "services/threadpool/async.h"
#include "textprocessor.h"
#include "config/engineconfiguration.h"
#include "defaultkeymanager/defaultkeymanager.h"

const std::string TextProcessor::objectPath_ =
    "/org/lingmo/aisdk/textprocessor";

std::set<int> TextProcessor::sessionIdSet;

namespace  {

std::map<ai_engine::lm::NlpEngineErrorCode, int> nlpEngineErrorMap {
    { ai_engine::lm::NlpEngineErrorCode::NomoreMemory, 3 },
    { ai_engine::lm::NlpEngineErrorCode::FileNotFound, 3 },
    { ai_engine::lm::NlpEngineErrorCode::VersionMismatched, 3 },
    { ai_engine::lm::NlpEngineErrorCode::FilePermissionDenied, 4 },
    { ai_engine::lm::NlpEngineErrorCode::NetworkDisconnected, 2 },
    { ai_engine::lm::NlpEngineErrorCode::NetworkLatencyTooLong, 2 },
    { ai_engine::lm::NlpEngineErrorCode::FailedToConnectServer, 3 },
    { ai_engine::lm::NlpEngineErrorCode::ServerNoResponse, 3 },
    { ai_engine::lm::NlpEngineErrorCode::NoServerService, 3 },
    { ai_engine::lm::NlpEngineErrorCode::Unauthorized, 4 },
    { ai_engine::lm::NlpEngineErrorCode::TooLittleParams, 7 },
    { ai_engine::lm::NlpEngineErrorCode::TooManyParams, 7 },
    { ai_engine::lm::NlpEngineErrorCode::InvalidParamType, 8 },
    { ai_engine::lm::NlpEngineErrorCode::TooLittleData, 7 },
    { ai_engine::lm::NlpEngineErrorCode::TooManyData, 7 },
    { ai_engine::lm::NlpEngineErrorCode::TooFrequentRequest, 5 },
    { ai_engine::lm::NlpEngineErrorCode::TooManyRequest, 5 },
    { ai_engine::lm::NlpEngineErrorCode::FailedToAnswer, 8 },
    { ai_engine::lm::NlpEngineErrorCode::InvalidWords, 8 },
};

}

TextProcessor::TextProcessor(GDBusConnection &connection,
    ai_engine::AiEnginePluginManager &aiEnginePluginManager)
    : connection_(connection)
    , aiEnginePluginManager_(aiEnginePluginManager) {
    exportSkeleton();
}

TextProcessor::~TextProcessor() {
    unexportSkeleton();
}

void TextProcessor::stopProcess()
{
    gintAndNlpEngineMap::iterator iter = nlpEngineMap_.begin();
    while (iter != nlpEngineMap_.end()) {
        iter->second->stopChat();
        ++iter;
    }

    std::lock_guard<std::mutex> locker(mutex_);
    stopped = true;
}

void TextProcessor::setSessionChatCallback(gint sessionId) {
    nlpEngineMap_.at(sessionId)->setChatResultCallback(
                [this, sessionId](ai_engine::lm::nlp::ChatResult result) {
        sendChatResult(sessionId, result);
    });
}

void TextProcessor::sendChatResult(gint sessionId, const ai_engine::lm::nlp::ChatResult &result)
{
    fprintf(stdout, "chat result: %s\n", result.text.c_str());
    bool tokenLimited{false};
    if (useDefaultKey_) {
        if (result.text.empty() &&
            DefaultKeyManager::instance().isNlpLimited()) {
            tokenLimited = true;
        }

        updateTokens(result.text);
    }

    {
        std::lock_guard<std::mutex> locker(mutex_);
        if (stopped) {
            return;
        }
    }

    std::string interfaceName = "org.lingmo.aisdk.textprocessor"
            + std::to_string((int)sessionId);
    fprintf(stdout, "intefaceName: %s\n", interfaceName.c_str());

    int errorCode = tokenLimited ? 4 : getSdkErrorCode(result.error.getCode());
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.text.c_str());
    g_variant_builder_add(&builder, "i", errorCode);

    std::shared_ptr<TextProcessor> textProcessor =
            static_cast<TextProcessor *>(this)->getSharedObject();
    g_dbus_connection_emit_signal(&(textProcessor->connection_),
                                    "org.lingmo.aisdk.textprocessor",
                                    "/org/lingmo/aisdk/textprocessor",
                                    interfaceName.c_str(),
                                    "ChatResult",
                                    g_variant_builder_end(&builder), nullptr);
}

void TextProcessor::updateTokens(const std::string &chatResult)
{
    // 从result.text中读取json数据
    Json::Value root;
    Json::Reader reader;
    bool newParseSuccess = reader.parse(chatResult, root);
    if (newParseSuccess) {
        std::string result = root["result"].asString();
        DefaultKeyManager::instance().updateNlpTokens(result);
    }
}

bool TextProcessor::onHandleInitEngine(AisdkTextProcessor *delegate,
    GDBusMethodInvocation *invocation, gchar *engineName, gchar *config, gpointer userData)
{
    TextProcessor *textProcessor = static_cast<TextProcessor *>(userData);
    if (!textProcessor) {
        fprintf(stderr, "init engine error: textProcessor is nullptr!\n");
        aisdk_text_processor_complete_init(delegate, invocation, 0,
            nlpEngineErrorMap.at(ai_engine::lm::NlpEngineErrorCode::NomoreMemory));
        return true;
    }

    std::fprintf(stdout, "service: init engine config: %s\n", engineName);
    std::string realEngineName{engineName};
    std::string realConfig{config};
    if (realEngineName == "default") {
        textProcessor->useDefaultKey_ = true;
        realEngineName = "xunfei";
        realConfig = DefaultKeyManager::instance().defaultNlpKey();
    } else {
        textProcessor->useDefaultKey_ = false;
    }

    textProcessor->promptTemplate_.reset(
        new config::PromptTemplate(config::promptTemplateConfigFile(realEngineName),
                                   config::promptFilePath(realEngineName))
    );

    int sessionId = textProcessor->generateRandomSessionId();
    bool initResult = textProcessor->initEngine(
                realEngineName, realConfig, sessionId);
    if (!initResult) {
        aisdk_text_processor_complete_init(delegate, invocation, sessionId,
            nlpEngineErrorMap.at(ai_engine::lm::NlpEngineErrorCode::NomoreMemory));
        return true;
    }

    int initModuleErrorCode = textProcessor->initSessionModule(sessionId);

    fprintf(stdout, "service: The sessionId of the engine is %d\n", sessionId);
    textProcessor->setSessionChatCallback(sessionId);
    aisdk_text_processor_complete_init(delegate, invocation, sessionId, initModuleErrorCode);
    return true;
}

bool TextProcessor::onHandleChat(
        AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
        gchar *question, gint sessionId, gpointer userData) {
    std::shared_ptr<TextProcessor> textProcessor = static_cast<TextProcessor *>(userData)->getSharedObject();
    if (!textProcessor ||
            textProcessor->nlpEngineMap_.find(sessionId) == textProcessor->nlpEngineMap_.end()) {
        fprintf(stderr, "handle chat error: textProcessor or nlpEngine is nullptr!\n");
        return true;
    }

    std::fprintf(stdout, "service: handle chat question: %s\n", question);

    if (textProcessor->useDefaultKey_) {
        if (DefaultKeyManager::instance().isNlpLimited()) {
            ai_engine::lm::nlp::ChatResult result;
            textProcessor->sendChatResult(sessionId, result);
            return true;
        }
        DefaultKeyManager::instance().updateNlpTokens(question);
    }

    std::string userQuestion = question;
    cpr::async([delegate, userQuestion, textProcessor, sessionId]() {
        std::string message;
        {
            std::lock_guard<std::mutex> locker(textProcessor->mutex_);
            message = textProcessor->actorChanged ?
                          textProcessor->currentPrompt_ + ":" + userQuestion : userQuestion;
            textProcessor->actorChanged = false;
        }

        ai_engine::lm::EngineError engineError;
        bool success = textProcessor->nlpEngineMap_.at(sessionId)->chat(
                    message, engineError);
        if (!success) {
            fprintf(stderr, "service error messages: %s!\n",
                    engineError.getMessage().c_str());
        }
    });

    return true;
}

bool TextProcessor::onSetActor(
        AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
        gint actor, gint sessionId, gpointer userData) {
    TextProcessor *textProcessor = static_cast<TextProcessor *>(userData);
    if (!textProcessor) {
        fprintf(stderr, "set actor error: textProcessor is nullptr!\n");
        return true;
    }

    if (actor == textProcessor->actorId_) {
        return true;
    }

    auto prompt = textProcessor->promptTemplate_->prompt((config::PromptTemplate::PromptId)actor);
    fprintf(stdout, "Prompt: %s\n", prompt.prompt.c_str());
    {
        std::lock_guard<std::mutex> locker(textProcessor->mutex_);
        textProcessor->actorChanged = true;
        textProcessor->currentPrompt_ = prompt.prompt;
    }
    return true;
}

bool TextProcessor::onStopChat(
        AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
        gint sessionId, gpointer userData) {
    TextProcessor *textProcessor = static_cast<TextProcessor *>(userData);
    if (!textProcessor ||
            textProcessor->nlpEngineMap_.find(sessionId) == textProcessor->nlpEngineMap_.end()) {
        fprintf(stderr, "stop chat error: textProcessor or nlpEngine is nullptr!\n");
        return true;
    }

    textProcessor->nlpEngineMap_.at(sessionId)->stopChat();
    return true;
}

bool TextProcessor::onSetContextSize(
        AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
        gint size, gint sessionId, gpointer userData) {
    TextProcessor *textProcessor = static_cast<TextProcessor *>(userData);
    if (!textProcessor ||
            textProcessor->nlpEngineMap_.find(sessionId) == textProcessor->nlpEngineMap_.end()) {
        fprintf(stderr, "set context size error: textProcessor or nlpEngine is nullptr!\n");
        return true;
    }

    textProcessor->nlpEngineMap_.at(sessionId)->setContextSize(size);
    return true;
}

bool TextProcessor::onClearContext(
        AisdkTextProcessor *delegate, GDBusMethodInvocation *invocation,
        gint sessionId, gpointer userData) {
    TextProcessor *textProcessor = static_cast<TextProcessor *>(userData);
    if (!textProcessor ||
            textProcessor->nlpEngineMap_.find(sessionId) == textProcessor->nlpEngineMap_.end()) {
        fprintf(stderr, "clear context error: textProcessor or nlpEngine is nullptr!\n");
        return true;
    }

    textProcessor->nlpEngineMap_.at(sessionId)->clearContext();
    return true;
}

void TextProcessor::exportSkeleton() {
    delegate_ = aisdk_text_processor_skeleton_new();

    g_signal_connect(delegate_, "handle-init",
                     G_CALLBACK(onHandleInitEngine), this);

    g_signal_connect(delegate_, "handle-chat",
                     G_CALLBACK(onHandleChat), this);

    g_signal_connect(delegate_, "handle-stop-chat",
                     G_CALLBACK(onStopChat), this);

    g_signal_connect(delegate_, "handle-set-actor",
                     G_CALLBACK(onSetActor), this);

    g_signal_connect(delegate_, "handle-set-context",
                     G_CALLBACK(onSetContextSize), this);

    g_signal_connect(delegate_, "handle-clear-context",
                     G_CALLBACK(onClearContext), this);

    GError *error = nullptr;

    isExported_ = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath_.c_str(),
        &error);
    if (!isExported_) {
        g_printerr("Error exporting skeleton at address %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
    }
}

void TextProcessor::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(
            G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    sessionIdSet.clear();
    g_object_unref(delegate_);
}

bool TextProcessor::initEngine(const std::string &engineName,
    const std::string &config, gint sessionId)
{
    auto nlpEngine = aiEnginePluginManager_.createNlpEngine(engineName);
    if (!nlpEngine) {
        fprintf(stderr, "initEngine nlpEngine is nullptr !\n");
        return false;
    }
    nlpEngineMap_[sessionId] = std::move(nlpEngine);

    if (nlpEngineMap_.at(sessionId)->isCloud()) {
        static_cast<ai_engine::lm::nlp::AbstractCloudNlpEngine *>(
                    nlpEngineMap_.at(sessionId).get())->setConfig(config);
    }
    return true;
}

int TextProcessor::initSessionModule(gint sessionId) {
    ai_engine::lm::EngineError initChatModuleEngineError;
    if (!nlpEngineMap_.at(sessionId)->initChatModule(initChatModuleEngineError)) {
        fprintf(stderr, "init chat module engine error!%d\n", initChatModuleEngineError.getCode());
        return getSdkErrorCode(initChatModuleEngineError.getCode());
    }
    return 0;
}

int TextProcessor::generateRandomSessionId() {
    int sessionId = std::rand();
    while (sessionIdSet.find(sessionId) != sessionIdSet.end()) {
        sessionId = std::rand();
    }
    sessionIdSet.insert(sessionId);
    return sessionId;
}

int TextProcessor::getSdkErrorCode(int engineErrorCode) {
    if (engineErrorCode == -1) {
        return 0;
    } else if (engineErrorCodeExists(engineErrorCode)) {
        return nlpEngineErrorMap.at((ai_engine::lm::NlpEngineErrorCode)engineErrorCode);
    } else {
        fprintf(stderr, "service error messages: speech engine error code is not exists!\n");
        return 0;
    }
}

bool TextProcessor::engineErrorCodeExists(int errorCode) {
    ai_engine::lm::NlpEngineErrorCode code =
            (ai_engine::lm::NlpEngineErrorCode)errorCode;
    return nlpEngineErrorMap.find(code) != nlpEngineErrorMap.end();
}
