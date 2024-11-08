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

#ifndef SERVICES_SPEECHPROCESSOR_H
#define SERVICES_SPEECHPROCESSOR_H

#include <set>
#include <string>
#include <memory>
#include <mutex>
#include <lingmo-ai/ai-engine/large-model/speechengine.h>
#include "services/speechprocessorglue.h"
#include "engine/aienginepluginmanager.h"
#include "config/prompttemplate.h"

class SpeechProcessor : public std::enable_shared_from_this<SpeechProcessor> {
public:
    explicit SpeechProcessor(
            GDBusConnection &connection,
            ai_engine::AiEnginePluginManager &aiEnginePluginManager);

    ~SpeechProcessor();

    void stopProcess();

private:
    void exportSkeleton();
    void connectSignal();
    void unexportSkeleton();
    int generateRandomSessionId();
    void printEngineErrorMessages(const ai_engine::lm::EngineError &engineError);
    int getSdkErrorCode(int engineErrorCode);
    bool engineErrorCodeExists(int errorCode);

    std::string base64decode(const std::string &input, const int dataLength);

    void emitLimitedRecognizingResult(int sessionId);
    void emitRecognizingResult(ai_engine::lm::speech::RecognitionResult result,
                               const gchar *interfaceName,
                               const gchar *signalName, bool isKeyLimited = false);

    void emitLimitedSynthesizingResult(int sessionId);
    void emitSynthesizingResult(ai_engine::lm::speech::SynthesisResult result,
                                const gchar *interfaceName,
                                const gchar *signalName, bool isKeyLimited = false);
    void emitStopSynthesizingAudioSignal(const gchar *interfaceName);

    bool initEngine(const std::string &engineName, const std::string &config, gint sessionId);
    int initSessionModule(gint sessionId);
    void setSessionRecognitionCallback(gint sessionId);
    void setSessionSynthesisCallback(gint sessionId);

    friend bool onHandleInitEngine(AisdkSpeechProcessor *delegate,
                                   GDBusMethodInvocation *invocation, gchar *engineName, gchar *config,
                                   gpointer userData);
    friend bool onHandleStartRecognizingAudio(AisdkSpeechProcessor *delegate,
                                              GDBusMethodInvocation *invocation,
                                              gchar *audioProperties, gint sessionId,
                                              gpointer userData);
    friend bool onHandleInputAudioData(AisdkSpeechProcessor *delegate,
                                       GDBusMethodInvocation *invocation,
                                       gchar *audioData, gint audioDataLength,
                                       gint sessionId, gpointer userData);
    friend bool onHandleStopRecognizingAudio(AisdkSpeechProcessor *delegate,
                                             GDBusMethodInvocation *invocation,
                                             gint sessionId, gpointer userData);

    friend bool onHandleRecognizingCompleteAudio(AisdkSpeechProcessor *delegate,
                                                 GDBusMethodInvocation *invocation,
                                                 gchar *audioProperties,
                                                 gchar *audioData,
                                                 gint audioDataLength,
                                                 gint sessionId,
                                                 gpointer userData);


    friend bool onHandleStartSynthesizingAudio(AisdkSpeechProcessor *delegate,
                                               GDBusMethodInvocation *invocation,
                                               gchar *voiceProperties, gint sessionId,
                                               gpointer userData);
    friend bool onHandleInputContinuousText(AisdkSpeechProcessor *delegate,
                                            GDBusMethodInvocation *invocation,
                                            gchar *text, gint sessionId,
                                            gpointer userData);
    friend bool onHandleStopSynthesizingAudio(AisdkSpeechProcessor *delegate,
                                              GDBusMethodInvocation *invocation,
                                              gint sessionId, gpointer userData);

    friend bool onHandleRecognizingCompleteText(AisdkSpeechProcessor *delegate,
                                                GDBusMethodInvocation *invocation,
                                                gchar *voiceProperties,
                                                gchar *text, gint sessionId,
                                                gpointer userData);

private:
    std::shared_ptr<SpeechProcessor> getSharedObject() {
        return shared_from_this();
    }

private:
    bool isExported_ = false;
    AisdkSpeechProcessor *delegate_ = nullptr;

    GDBusConnection &connection_;

    static const std::string objectPath_;

    ai_engine::AiEnginePluginManager &aiEnginePluginManager_;
    std::mutex mutex_;
    bool stopped { false };

    static std::set<int> sessionIdSet_;

    typedef std::map<gint,
    std::unique_ptr<ai_engine::lm::speech::AbstractSpeechEngine>> gintAndSpeechEngineMap;
    gintAndSpeechEngineMap speechEngineMap_;

    bool useDefaultKey_ { false };
};

#endif
