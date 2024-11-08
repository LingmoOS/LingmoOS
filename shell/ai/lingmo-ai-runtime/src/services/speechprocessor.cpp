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

#include "services/speechprocessor.h"

#include <chrono>
#include <cstdio>
#include <lingmo-ai/ai-engine/large-model/cloudspeechengine.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "services/threadpool/async.h"
#include "defaultkeymanager/defaultkeymanager.h"

const std::string SpeechProcessor::objectPath_ =
    "/org/lingmo/aisdk/speechprocessor";
std::set<int> SpeechProcessor::sessionIdSet_;

namespace  {

std::chrono::high_resolution_clock::time_point startTime;

std::map<ai_engine::lm::SpeechEngineErrorCode, int> speechEngineErrorMap {
    { ai_engine::lm::SpeechEngineErrorCode::NomoreMemory, 3 },
    { ai_engine::lm::SpeechEngineErrorCode::FileNotFound, 3 },
    { ai_engine::lm::SpeechEngineErrorCode::VersionMismatched, 3 },
    { ai_engine::lm::SpeechEngineErrorCode::FilePermissionDenied, 4 },
    { ai_engine::lm::SpeechEngineErrorCode::NetworkDisconnected, 2 },
    { ai_engine::lm::SpeechEngineErrorCode::NetworkLatencyTooLong, 2 },
    { ai_engine::lm::SpeechEngineErrorCode::FailedToConnectServer, 3 },
    { ai_engine::lm::SpeechEngineErrorCode::ServerNoResponse, 3 },
    { ai_engine::lm::SpeechEngineErrorCode::NoServerService, 3 },
    { ai_engine::lm::SpeechEngineErrorCode::Unauthorized, 4 },
    { ai_engine::lm::SpeechEngineErrorCode::TooLittleParams, 7 },
    { ai_engine::lm::SpeechEngineErrorCode::TooManyParams, 7 },
    { ai_engine::lm::SpeechEngineErrorCode::InvalidParamType, 9 },
    { ai_engine::lm::SpeechEngineErrorCode::TooLittleData, 8 },
    { ai_engine::lm::SpeechEngineErrorCode::TooManyData, 8 },
    { ai_engine::lm::SpeechEngineErrorCode::TooFrequentRequest, 5 },
    { ai_engine::lm::SpeechEngineErrorCode::TooManyRequest, 5 },
    { ai_engine::lm::SpeechEngineErrorCode::UnsupportedLanguage, 9 },
    { ai_engine::lm::SpeechEngineErrorCode::UnsupportedScript, 9 },
    { ai_engine::lm::SpeechEngineErrorCode::InvalidWakeupWords, 9 }
};

}


SpeechProcessor::SpeechProcessor(
        GDBusConnection &connection,
        ai_engine::AiEnginePluginManager &aiEnginePluginManager)
    : connection_(connection), aiEnginePluginManager_(aiEnginePluginManager) {
    exportSkeleton();
}

SpeechProcessor::~SpeechProcessor() { unexportSkeleton(); }

void SpeechProcessor::stopProcess() {
    gintAndSpeechEngineMap::iterator iter = speechEngineMap_.begin();
    while (iter != speechEngineMap_.end()) {
        ai_engine::lm::EngineError stopSynthesisEngineError;
        if (!iter->second->stopContinuousSynthesis(stopSynthesisEngineError)) {
            printEngineErrorMessages(stopSynthesisEngineError);
        }

        ai_engine::lm::EngineError stopRecognitionEngineError;
        if (!iter->second->stopContinuousRecognition(stopRecognitionEngineError)) {
            printEngineErrorMessages(stopRecognitionEngineError);
        }
        ++iter;
    }
    std::lock_guard<std::mutex> locker(mutex_);
    stopped = true;
}

void SpeechProcessor::exportSkeleton() {
    delegate_ = aisdk_speech_processor_skeleton_new();
    connectSignal();

    GError *error = nullptr;

    isExported_ = g_dbus_interface_skeleton_export(
        G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath_.c_str(),
        &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n",
                   objectPath_.c_str(), error->message);
        g_error_free(error);
    }
}

std::string SpeechProcessor::base64decode(const std::string &input,
                                          const int dataLength) {
    BIO *bio, *b64;
    char *buffer = new char[dataLength];
    memset(buffer, 0, dataLength);

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(input.c_str(), input.length());
    bio = BIO_push(b64, bio);

    BIO_read(bio, buffer, input.length());

    BIO_free_all(bio);

    std::string result(buffer, buffer + dataLength);

    delete[] buffer;
    return result;
}

void SpeechProcessor::emitLimitedRecognizingResult(int sessionId)
{
    cpr::async([this, sessionId]() {
        std::string interfaceName = "org.lingmo.aisdk.speechprocessor"
            + std::to_string((int)sessionId);
        emitRecognizingResult(
            ai_engine::lm::speech::RecognitionResult {},
            interfaceName.c_str(), "RecognizingAudioResult", true);
        emitRecognizingResult(
            ai_engine::lm::speech::RecognitionResult {},
            interfaceName.c_str(), "StopRecognizingAudioResult", true);
    });

}
void SpeechProcessor::emitRecognizingResult(
        ai_engine::lm::speech::RecognitionResult result,
        const gchar *interfaceName,
        const gchar *signalName,
        bool isKeyLimited) {

    int errorCode = isKeyLimited ? 4 : getSdkErrorCode(result.error.getCode());
    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.text.c_str());
    g_variant_builder_add(&builder, "i", result.text.length());
    g_variant_builder_add(&builder, "i", result.speakerId);
    g_variant_builder_add(&builder, "i", errorCode);

    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(this)->getSharedObject();

    g_dbus_connection_emit_signal(&(speechProcessor->connection_),
                                  "org.lingmo.aisdk.speechprocessor",
                                  "/org/lingmo/aisdk/speechprocessor",
                                  interfaceName,
                                  signalName,
                                  g_variant_builder_end(&builder),
                                  nullptr);
}


void SpeechProcessor::emitLimitedSynthesizingResult(int sessionId) {
    cpr::async([this, sessionId]() {
        std::string interfaceName = "org.lingmo.aisdk.speechprocessor"
            + std::to_string((int)sessionId);
        emitSynthesizingResult(
            ai_engine::lm::speech::SynthesisResult {},
            interfaceName.c_str(), "SynthesizingAudioResult", true);

        emitSynthesizingResult(
            ai_engine::lm::speech::SynthesisResult {},
            interfaceName.c_str(), "StopSynthesizingAudioResult", true);
    });
}
void SpeechProcessor::emitSynthesizingResult(
        ai_engine::lm::speech::SynthesisResult result,
        const gchar *interfaceName,
        const gchar *signalName,
        bool isKeyLimited) {

    int errorCode = isKeyLimited ? 4 : getSdkErrorCode(result.error.getCode());
    // 创建包含字节数组的 GVariant
    GVariant *byte_array_variant = g_variant_new_from_data(
        G_VARIANT_TYPE_BYTESTRING,
        result.audioData.data(),
        result.audioData.size(),
        TRUE, NULL, NULL);

    GVariantBuilder builder;
    g_variant_builder_init (&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_open (&builder, G_VARIANT_TYPE_ARRAY);
    g_variant_builder_add_value(&builder, byte_array_variant);
    g_variant_builder_close (&builder);

    g_variant_builder_add(&builder, "s", result.audioFormat.c_str());
    g_variant_builder_add(&builder, "i", result.audioRate);
    g_variant_builder_add(&builder, "i", result.audioChannel);
    g_variant_builder_add(&builder, "i", errorCode);

    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(this)->getSharedObject();

    g_dbus_connection_emit_signal(&(speechProcessor->connection_),
                                  "org.lingmo.aisdk.speechprocessor",
                                  "/org/lingmo/aisdk/speechprocessor",
                                  interfaceName,
                                  signalName,
                                  g_variant_builder_end(&builder),
                                  nullptr);
}

void SpeechProcessor::emitStopSynthesizingAudioSignal(const gchar *interfaceName) {
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(this)->getSharedObject();

    g_dbus_connection_emit_signal(&(speechProcessor->connection_),
                                  "org.lingmo.aisdk.speechprocessor",
                                  "/org/lingmo/aisdk/speechprocessor",
                                  interfaceName,
                                  "StopSynthesizingAudioResult",
                                  nullptr,
                                  nullptr);
}

bool SpeechProcessor::initEngine(const std::string &engineName, const std::string &config,
                                 gint sessionId) {
    auto speechEngine = aiEnginePluginManager_.createSpeechEngine(engineName);
    if (!speechEngine) {
        fprintf(stderr, "service error messages: initEngine speechEngine is nullptr!\n");
        return false;
    }
    speechEngineMap_[sessionId] = std::move(speechEngine);
    if (speechEngineMap_.at(sessionId)->isCloud()) {
        static_cast<ai_engine::lm::speech::AbstractCloudSpeechEngine *>(
                    speechEngineMap_.at(sessionId).get())->setConfig(config);
    }
    return true;
}

int SpeechProcessor::initSessionModule(gint sessionId) {
    bool initContinuousRecognitionModuleResult;
    bool initRecognizeOnceModuleResult;
    bool initContinuousSynthesisModuleResult;
    bool initSynthesizeOnceModuleResult;
    ai_engine::lm::EngineError initModuleEngineError;

    ai_engine::lm::EngineError initContinuousRecognitionModuleEngineError;
    initContinuousRecognitionModuleResult = speechEngineMap_.at(sessionId)
        ->initContinuousRecognitionModule(initContinuousRecognitionModuleEngineError);
    if (!initContinuousRecognitionModuleResult) {
        fprintf(stderr, "init continuous recognition module engine error!\n");
        initModuleEngineError = initContinuousRecognitionModuleEngineError;
    }

    ai_engine::lm::EngineError initRecognizeOnceModuleEngineError;
    initRecognizeOnceModuleResult = speechEngineMap_.at(sessionId)
        ->initRecognizeOnceModule(initRecognizeOnceModuleEngineError);
    if (!initRecognizeOnceModuleResult) {
        fprintf(stderr, "init recognition once module engine error!\n");
        initModuleEngineError = initRecognizeOnceModuleEngineError;
    }

    ai_engine::lm::EngineError initContinuousSynthesisModuleEngineError;
    initContinuousSynthesisModuleResult = speechEngineMap_.at(sessionId)
        ->initContinuousSynthesisModule(initContinuousSynthesisModuleEngineError);
    if (!initContinuousSynthesisModuleResult) {
        fprintf(stderr, "init continuous synthesize module engine error!\n");
        initModuleEngineError = initContinuousSynthesisModuleEngineError;
    }

    ai_engine::lm::EngineError initSynthesizeOnceModuleEngineError;
    initSynthesizeOnceModuleResult = speechEngineMap_.at(sessionId)
        ->initSynthesizeOnceModule(initSynthesizeOnceModuleEngineError);
    if (!initSynthesizeOnceModuleResult) {
        fprintf(stderr, "init synthesize once module engine error!\n");
        initModuleEngineError = initSynthesizeOnceModuleEngineError;
    }

    if (initContinuousRecognitionModuleResult || initRecognizeOnceModuleResult
            || initContinuousSynthesisModuleResult || initSynthesizeOnceModuleResult) {
        return 0;
    }
    return getSdkErrorCode(initModuleEngineError.getCode());
}

void SpeechProcessor::setSessionRecognitionCallback(gint sessionId) {
    std::string interfaceName = "org.lingmo.aisdk.speechprocessor"
            + std::to_string((int)sessionId);

    speechEngineMap_.at(sessionId)->setRecognizingCallback(
                [this, interfaceName](ai_engine::lm::speech::RecognitionResult result){
        {
            std::lock_guard<std::mutex> locker(mutex_);
            if (stopped) {
                return;
            }
        }
        fprintf(stdout, "interfaceName: %s\n", interfaceName.c_str());
        fprintf(stdout, "recognition result: %s\n", result.text.c_str());
        emitRecognizingResult(result, interfaceName.c_str(), "RecognizingAudioResult");
    });

    speechEngineMap_.at(sessionId)->setRecognizedCallback(
                [this, interfaceName](ai_engine::lm::speech::RecognitionResult result){
        {
            std::lock_guard<std::mutex> locker(mutex_);
            if (stopped) {
                return;
            }
        }
        fprintf(stdout, "interfaceName: %s\n", interfaceName.c_str());
        fprintf(stdout, "recognition result: %s\n", result.text.c_str());
        emitRecognizingResult(result, interfaceName.c_str(), "StopRecognizingAudioResult");
    });
}

void SpeechProcessor::setSessionSynthesisCallback(gint sessionId) {
    std::string interfaceName = "org.lingmo.aisdk.speechprocessor"
            + std::to_string((int)sessionId);

    speechEngineMap_.at(sessionId)->setSynthesizingCallback(
                [this, interfaceName](ai_engine::lm::speech::SynthesisResult result){
        {
            std::lock_guard<std::mutex> locker(mutex_);
            if (stopped) {
                return;
            }
        }
        fprintf(stdout, "interfaceName: %s\n", interfaceName.c_str());
        fprintf(stdout, "synthesis result size: %d\n", result.audioData.size());
        emitSynthesizingResult(result, interfaceName.c_str(), "SynthesizingAudioResult");
    });

    speechEngineMap_.at(sessionId)->setSynthesizedCallback(
                [this, interfaceName](ai_engine::lm::speech::ResultType result) {
        fprintf(stdout, "interfaceName: %s\n", interfaceName.c_str());

        emitStopSynthesizingAudioSignal(interfaceName.c_str());
    });
}

bool onHandleInitEngine(AisdkSpeechProcessor *delegate,
                        GDBusMethodInvocation *invocation, gchar *engineName, gchar *config,
                        gpointer userData) {
    auto *speechProcessor = static_cast<SpeechProcessor *>(userData);
    if (!speechProcessor) {
        fprintf(stderr, "init engine error: speechProcessor is nullptr!\n");
        aisdk_speech_processor_complete_init(delegate, invocation, 0,
            speechEngineErrorMap.at(ai_engine::lm::SpeechEngineErrorCode::NomoreMemory));
        return true;
    }

    std::string realEngineName{engineName};
    std::string realConfig{config};
    if (realEngineName == "default") {
        speechProcessor->useDefaultKey_ = true;
        realEngineName = "xunfei";
        realConfig = DefaultKeyManager::instance().defaultSpeechKey();
    } else {
        speechProcessor->useDefaultKey_ = false;
    }

    int sessionId = speechProcessor->generateRandomSessionId();
    bool initResult = speechProcessor->initEngine(
                realEngineName, realConfig, sessionId);
    if (!initResult) {
        fprintf(stderr, "engine initialization failed!\n");
        aisdk_speech_processor_complete_init(delegate, invocation, sessionId,
            speechEngineErrorMap.at(ai_engine::lm::SpeechEngineErrorCode::NomoreMemory));
        return true;
    }

    int initModuleErrorCode = speechProcessor->initSessionModule(sessionId);

    fprintf(stdout, "the sessionId of the engine is %d\n", sessionId);
    speechProcessor->setSessionRecognitionCallback(sessionId);
    speechProcessor->setSessionSynthesisCallback(sessionId);
    aisdk_speech_processor_complete_init(delegate, invocation, sessionId, initModuleErrorCode);
    return true;
}

bool onHandleStartRecognizingAudio(AisdkSpeechProcessor *delegate,
                                   GDBusMethodInvocation *invocation,
                                   gchar *audioProperties, gint sessionId,
                                   gpointer userData) {
    auto *speechProcessor = static_cast<SpeechProcessor *>(userData);
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "start recognize error: speechProcessor or speechEngine is nullptr!\n");
        aisdk_speech_processor_complete_start_recognizing_audio(delegate, invocation,
            speechEngineErrorMap.at(ai_engine::lm::SpeechEngineErrorCode::NomoreMemory));
        return true;
    }

    if (speechProcessor->useDefaultKey_) {
        if (DefaultKeyManager::instance().isRealTimeSpeechRecongnitionLimit()) {
            speechProcessor->emitLimitedRecognizingResult(sessionId);
            return true;
        }
        startTime = std::chrono::system_clock::now();
    }

    ai_engine::lm::EngineError engineError;
    bool success = speechProcessor->speechEngineMap_.at(sessionId)
            ->startContinuousRecognition(audioProperties, engineError);
    if (!success) {
        speechProcessor->printEngineErrorMessages(engineError);
        aisdk_speech_processor_complete_start_recognizing_audio(delegate, invocation,
            speechEngineErrorMap.at(ai_engine::lm::SpeechEngineErrorCode(engineError.getCode())));
        return true;
    }
    aisdk_speech_processor_complete_start_recognizing_audio(delegate, invocation, 0);

    return true;
}

bool onHandleInputAudioData(AisdkSpeechProcessor *delegate,
                            GDBusMethodInvocation *invocation,
                            gchar *audioData, gint audioDataLength,
                            gint sessionId, gpointer userData) {
    //audioData是编码后的数据
    //audioDataLength是原始数据的数据长度
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(userData)->getSharedObject();
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "write continue audio data error: speechProcessor or speechEngine is nullptr!\n");
        return true;
    }

    if (speechProcessor->useDefaultKey_ &&
        DefaultKeyManager::instance().isRealTimeSpeechRecongnitionLimit()) {
        speechProcessor->emitLimitedRecognizingResult(sessionId);
        return true;
    }

    std::string data { audioData };
    std::string decodeData = speechProcessor->base64decode(data, audioDataLength);
    std::vector<char> inputData { decodeData.data(), decodeData.data() + decodeData.size() };

    auto future = cpr::async([speechProcessor, inputData, sessionId]() {
        ai_engine::lm::EngineError engineError;
        bool success = speechProcessor->speechEngineMap_.at(sessionId)
                ->writeContinuousRecognitionAudioData(inputData, engineError);
        if (!success) {
            speechProcessor->printEngineErrorMessages(engineError);
        }
    });

    future.wait();
    return true;
}

bool onHandleStopRecognizingAudio(AisdkSpeechProcessor *delegate,
                                  GDBusMethodInvocation *invocation,
                                  gint sessionId, gpointer userData) {
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(userData)->getSharedObject();
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "stop recognizing error: speechProcessor or speechEngine is nullptr!\n");
        return true;
    }

    cpr::async([speechProcessor, sessionId]() {
        ai_engine::lm::EngineError engineError;
        bool success = speechProcessor->speechEngineMap_.at(sessionId)
                ->stopContinuousRecognition(engineError);
        if (!success) {
            speechProcessor->printEngineErrorMessages(engineError);
        }
    });

    if (speechProcessor->useDefaultKey_) {
        auto endTime = std::chrono::high_resolution_clock::now();
        double timeSpent = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
        std::fprintf(stderr, "Time spent: %f\n", timeSpent);
        DefaultKeyManager::instance().updateRealTimeSpeechRecongnitionTime((int)timeSpent);
    }

    return true;
}

bool onHandleRecognizingCompleteAudio(AisdkSpeechProcessor *delegate,
                                      GDBusMethodInvocation *invocation,
                                      gchar *audioProperties,
                                      gchar *audioData,
                                      gint audioDataLength,
                                      gint sessionId,
                                      gpointer userData) {
    //audioData是编码后的数据
    //audioDataLength是原始数据的数据长度
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(userData)->getSharedObject();
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "recognizing complete audio once error: speechProcessor or speechEngine is nullptr!\n");
        return true;
    }

    if (speechProcessor->useDefaultKey_ &&
        DefaultKeyManager::instance().isOnceSpeechRecognitionLimit()) {
        speechProcessor->emitLimitedRecognizingResult(sessionId);
        return true;
    }

    std::string data { audioData };
    std::string decodeData = speechProcessor->base64decode(data, audioDataLength);
    std::vector<char> inputData { decodeData.data(), decodeData.data() + decodeData.size() };
    std::string params { audioProperties };

    cpr::async([speechProcessor, inputData, params, sessionId]() {
        ai_engine::lm::EngineError engineError;
        bool success = speechProcessor->speechEngineMap_.at(sessionId)
                ->recognizeOnce(params, inputData, engineError);
        if (!success) {
            speechProcessor->printEngineErrorMessages(engineError);
        }
    });

    if (speechProcessor->useDefaultKey_) {
        DefaultKeyManager::instance().updateOnceSpeechRecognitionCount(1);
    }

    return true;
}

bool onHandleStartSynthesizingAudio(AisdkSpeechProcessor *delegate,
                                    GDBusMethodInvocation *invocation,
                                    gchar *voiceProperties,
                                    gint sessionId, gpointer userData) {
    auto *speechProcessor = static_cast<SpeechProcessor *>(userData);
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "start continuous synthesis error: speechProcessor or speechEngine is nullptr!\n");
        aisdk_speech_processor_complete_start_synthesizing_audio(delegate, invocation,
            speechEngineErrorMap.at(ai_engine::lm::SpeechEngineErrorCode::NomoreMemory));
        return true;
    }

    if (speechProcessor->useDefaultKey_ &&
        DefaultKeyManager::instance().isSynthesizingLimit()) {
        speechProcessor->emitLimitedSynthesizingResult(sessionId);
        return true;
    }

    ai_engine::lm::EngineError engineError;
    bool success = speechProcessor->speechEngineMap_.at(sessionId)
            ->startContinuousSynthesis(voiceProperties, engineError);
    if (!success) {
        speechProcessor->printEngineErrorMessages(engineError);
        aisdk_speech_processor_complete_start_synthesizing_audio(delegate, invocation,
            speechEngineErrorMap.at(ai_engine::lm::SpeechEngineErrorCode(engineError.getCode())));
        return true;
    }
    aisdk_speech_processor_complete_start_synthesizing_audio(delegate, invocation, 0);

    if (speechProcessor->useDefaultKey_) {
        DefaultKeyManager::instance().updateSynthesizingCount(1);
    }

    return true;
}

bool onHandleInputContinuousText(AisdkSpeechProcessor *delegate,
                                 GDBusMethodInvocation *invocation,
                                 gchar *text, gint sessionId,
                                 gpointer userData) {
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(userData)->getSharedObject();
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "write continuous synthesis text error: speechProcessor or speechEngine is nullptr!\n");
        return true;
    }

    if (speechProcessor->useDefaultKey_ &&
        DefaultKeyManager::instance().isSynthesizingLimit()) {
        speechProcessor->emitLimitedSynthesizingResult(sessionId);
        return true;
    }


    std::string textString { text };

    cpr::async([speechProcessor, textString, sessionId]() {
        ai_engine::lm::EngineError engineError;
        bool success = speechProcessor->speechEngineMap_.at(sessionId)
                ->writeContinuousSynthesisText(textString, engineError);
        if (!success) {
            speechProcessor->printEngineErrorMessages(engineError);
        }
    });

    return true;
}

bool onHandleStopSynthesizingAudio(AisdkSpeechProcessor *delegate,
                                   GDBusMethodInvocation *invocation,
                                   gint sessionId, gpointer userData) {
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(userData)->getSharedObject();
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "stop continuous synthesis error: speechProcessor or speechEngine is nullptr!\n");
        return true;
    }

    ai_engine::lm::EngineError engineError;
    bool success = speechProcessor->speechEngineMap_.at(sessionId)
            ->stopContinuousSynthesis(engineError);
    if (!success) {
        speechProcessor->printEngineErrorMessages(engineError);
    }
    return true;
}

bool onHandleRecognizingCompleteText(AisdkSpeechProcessor *delegate,
                                     GDBusMethodInvocation *invocation,
                                     gchar *voiceProperties,
                                     gchar *text, gint sessionId,
                                     gpointer userData) {
    std::shared_ptr<SpeechProcessor> speechProcessor =
            static_cast<SpeechProcessor *>(userData)->getSharedObject();
    if (!speechProcessor ||
            speechProcessor->speechEngineMap_.find(sessionId)
            == speechProcessor->speechEngineMap_.end()) {
        fprintf(stderr, "synthesize once error: speechProcessor or speechEngine is nullptr!\n");
        return true;
    }

    if (speechProcessor->useDefaultKey_ &&
        DefaultKeyManager::instance().isSynthesizingLimit()) {
        speechProcessor->emitLimitedSynthesizingResult(sessionId);
        return true;
    }

    std::string textString { text };

    cpr::async([speechProcessor, voiceProperties, textString, sessionId]() {
        ai_engine::lm::EngineError engineError;
        bool success = speechProcessor->speechEngineMap_.at(sessionId)
                ->synthesizeOnce(voiceProperties, textString, engineError);
        if (!success) {
            speechProcessor->printEngineErrorMessages(engineError);
        }
    });

    if (speechProcessor->useDefaultKey_) {
        DefaultKeyManager::instance().updateSynthesizingCount(1);
    }

    return true;
}

void SpeechProcessor::connectSignal() {
    g_signal_connect(delegate_, "handle-init",
                     G_CALLBACK(onHandleInitEngine),
                     this);

    g_signal_connect(delegate_, "handle-start-recognizing-audio",
                     G_CALLBACK(onHandleStartRecognizingAudio),
                     this);
    g_signal_connect(delegate_, "handle-input-audio-data",
                     G_CALLBACK(onHandleInputAudioData),
                     this);
    g_signal_connect(delegate_, "handle-stop-recognizing-audio",
                     G_CALLBACK(onHandleStopRecognizingAudio),
                     this);
    g_signal_connect(delegate_, "handle-recognizing-complete-audio",
                     G_CALLBACK(onHandleRecognizingCompleteAudio),
                     this);

    g_signal_connect(delegate_, "handle-start-synthesizing-audio",
                     G_CALLBACK(onHandleStartSynthesizingAudio),
                     this);
    g_signal_connect(delegate_, "handle-input-continuous-text",
                     G_CALLBACK(onHandleInputContinuousText),
                     this);
    g_signal_connect(delegate_, "handle-stop-synthesizing-audio",
                     G_CALLBACK(onHandleStopSynthesizingAudio),
                     this);
    g_signal_connect(delegate_, "handle-recognizing-complete-text",
                     G_CALLBACK(onHandleRecognizingCompleteText),
                     this);
}

void SpeechProcessor::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(
            G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    sessionIdSet_.clear();
    g_object_unref(delegate_);
}

int SpeechProcessor::generateRandomSessionId() {
    int sessionId = std::rand();
    while (sessionIdSet_.find(sessionId) != sessionIdSet_.end()) {
        sessionId = std::rand();
    }
    sessionIdSet_.insert(sessionId);
    return sessionId;
}

void SpeechProcessor::printEngineErrorMessages(
        const ai_engine::lm::EngineError &engineError) {
    fprintf(stderr, "service error messages: %s!\n", engineError.getMessage().c_str());
}

int SpeechProcessor::getSdkErrorCode(int engineErrorCode) {
    if (engineErrorCode == -1) {
        return 0;
    } else if (engineErrorCodeExists(engineErrorCode)) {
        return speechEngineErrorMap.at((ai_engine::lm::SpeechEngineErrorCode)engineErrorCode);
    } else {
        fprintf(stderr, "get sdk code error: speech engine error code is not exists!\n");
        return 0;
    }
}

bool SpeechProcessor::engineErrorCodeExists(int errorCode) {
    ai_engine::lm::SpeechEngineErrorCode code =
            (ai_engine::lm::SpeechEngineErrorCode)errorCode;
    return speechEngineErrorMap.find(code) != speechEngineErrorMap.end();
}


