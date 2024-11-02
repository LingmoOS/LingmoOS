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

#ifndef SERVICES_SPEECHPROCESSORPROXY_H
#define SERVICES_SPEECHPROCESSORPROXY_H

#include <functional>
#include <jsoncpp/json/value.h>
#include <string>

#include "services/speechprocessorglue.h"
#include <lingmo-ai/ai-base/speech.h>

class SpeechProcessorProxy {
public:
    using SpeechRecognitionResultCallback = std::function<void(RecognitionResult result)>;
    using SpeechSynthesisResultCallback = std::function<void(SynthesisResult result)>;
    using SpeechFinishedCallback = std::function<void()>;

    SpeechProcessorProxy();
    ~SpeechProcessorProxy();

    void setRecognizingAudioCallback(
            SpeechProcessorProxy::SpeechRecognitionResultCallback callback) {
        recognizingAudioCallback_ = callback;
    }
    void setStopRecognizingAudioCallback(
            SpeechProcessorProxy::SpeechRecognitionResultCallback callback) {
        stopRecognizingAudioCallback_ = callback;
    }

    void setSynthesizingAudioCallback(
            SpeechProcessorProxy::SpeechSynthesisResultCallback callback) {
        synthesizingAudioCallback_ = callback;
    }
    void setStopSynthesizingAudioCallback(
            SpeechProcessorProxy::SpeechFinishedCallback callback) {
        stopSynthesizingAudioCallback_ = callback;
    }

    SpeechResult initEngine(const std::string &engineName, const Json::Value &config,
                            gpointer userData);
    SpeechResult startRecognizingAudio(const char *audioProperties);
    SpeechResult inputAudioData(const char *audioData,
                                int audioDataLength);
    SpeechResult stopRecognizingAudio();
    SpeechResult recognizingCompleteAudio(const char *audioProperties,
                                          const char *audioData,
                                          int audioDataLength);

    SpeechResult startSynthesizingAudio(const char *voiceProperties);
    SpeechResult inputContinuousText(const char *text);
    SpeechResult stopSynthesizingAudio();

    SpeechResult recognizingCompleteText(const char *voiceProperties,
                                         const char *text);

private:
    void init();
    void destroy();
    void createDelegate();
    void connectSignals();
    std::string base64Encode(const std::string &input);

    friend RecognitionResult recognitionResult(GVariant *parameters);
    friend SynthesisResult synthesisResult(GVariant *parameters);
    friend void onRecognizingAudioResult(GDBusConnection *connection,
                                         const gchar *sender_name,
                                         const gchar *object_path,
                                         const gchar *interface_name,
                                         const gchar *signal_name,
                                         GVariant *parameters,
                                         gpointer user_data);
    friend void onStopRecognizingAudioResult(GDBusConnection *connection,
                                             const gchar *sender_name,
                                             const gchar *object_path,
                                             const gchar *interface_name,
                                             const gchar *signal_name,
                                             GVariant *parameters,
                                             gpointer user_data);
    friend void onSynthesizingAudioResult(GDBusConnection *connection,
                                          const gchar *sender_name,
                                          const gchar *object_path,
                                          const gchar *interface_name,
                                          const gchar *signal_name,
                                          GVariant *parameters,
                                          gpointer user_data);
    friend void onStopSynthesizingAudioResult(GDBusConnection *connection,
                                              const gchar *sender_name,
                                              const gchar *object_path,
                                              const gchar *interface_name,
                                              const gchar *signal_name,
                                              GVariant *parameters,
                                              gpointer user_data);

private:
    AisdkSpeechProcessor *delegate_ = nullptr;

    SpeechRecognitionResultCallback recognizingAudioCallback_ = nullptr;
    SpeechRecognitionResultCallback stopRecognizingAudioCallback_ = nullptr;

    SpeechSynthesisResultCallback synthesizingAudioCallback_ = nullptr;
    SpeechFinishedCallback stopSynthesizingAudioCallback_ = nullptr;

    const char *objectPath_ = "/org/lingmo/aisdk/speechprocessor";
    const char *interfaceName_ = "org.lingmo.aisdk.speechprocessor";

    int sessionId_;
    int initErrorCode_; //0表示初始化成功，非0表示初始化失败
};
#endif
