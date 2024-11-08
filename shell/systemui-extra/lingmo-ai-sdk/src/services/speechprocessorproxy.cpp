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

#include "services/speechprocessorproxy.h"

#include <iostream>
#include <jsoncpp/json/value.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <iomanip>
#include "services/serverproxy.h"

RecognitionResult recognitionResult(GVariant *parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *char_text = g_variant_iter_next_value(&iter);
    GVariant *int_length = g_variant_iter_next_value(&iter);
    GVariant *int_speaker = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    if (!char_text || !int_length || !int_speaker || !int_error) {
        fprintf(stderr, "recognition result error: result is nullptr!\n");
        return RecognitionResult {};
    }

    char *text = (char*)g_variant_get_data(char_text);
    int *length = (int *)g_variant_get_data(int_length);
    int *speaker = (int *)g_variant_get_data(int_speaker);
    int *error = (int *)g_variant_get_data(int_error);

    if (!text || !length || !speaker || !error) {
        fprintf(stderr, "recognition result error: result is nullptr!\n");
        return RecognitionResult {};
    }
    RecognitionResult recognitionResult {
        text, (unsigned int)(*length), *speaker, *error };
    return recognitionResult;
}

SynthesisResult synthesisResult(GVariant *parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *array_data = g_variant_iter_next_value(&iter); // 获取字节数组
    GVariant *int_audioFormat = g_variant_iter_next_value(&iter); // 获取整数
    GVariant *int_audioRate = g_variant_iter_next_value(&iter);
    GVariant *int_audioChannel = g_variant_iter_next_value(&iter);
    GVariant *int_error = g_variant_iter_next_value(&iter);

    if (!array_data || !int_audioFormat || !int_audioRate
            || !int_audioChannel || !int_error) {
        fprintf(stderr, "synthesis result error: result is nullptr!\n");
        return SynthesisResult {};
    }

    char* audioFormat = (char*)g_variant_get_data(int_audioFormat);
    int* audioRate = (int*)g_variant_get_data(int_audioRate);
    int* audioChannel = (int*)g_variant_get_data(int_audioChannel);
    int* error = (int*)g_variant_get_data(int_error);

    if (!audioRate || !audioChannel || !error) {
        fprintf(stderr, "synthesis result error: result is nullptr!\n");
        return SynthesisResult {};
    }

    gchar *arr = (gchar*)g_variant_get_data(array_data);

    std::vector<char> vec;
    if (g_variant_get_size(array_data) > 2) {
        vec = { arr, arr + g_variant_get_size(array_data) - 2 };
    }
    SynthesisResult synthesisResult {
        audioFormat, *audioRate, *audioChannel, (unsigned char*)vec.data(),
        (unsigned int)vec.size(), *error };
    return synthesisResult;
}

void onRecognizingAudioResult(GDBusConnection *connection,
                              const gchar *sender_name,
                              const gchar *object_path,
                              const gchar *interface_name,
                              const gchar *signal_name,
                              GVariant *parameters,
                              gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->recognizingAudioCallback_ == nullptr) {
        std::cout << "RecognitionAudioResult callback is nullptr" << std::endl;
        return;
    }
    proxy->recognizingAudioCallback_(recognitionResult(parameters));
}

void onStopRecognizingAudioResult(GDBusConnection *connection,
                                  const gchar *sender_name,
                                  const gchar *object_path,
                                  const gchar *interface_name,
                                  const gchar *signal_name,
                                  GVariant *parameters,
                                  gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->stopRecognizingAudioCallback_ == nullptr) {
        std::cout << "StopRecognitionAudioResult callback is nullptr" << std::endl;
        return;
    }
    proxy->stopRecognizingAudioCallback_(recognitionResult(parameters));
}


void onSynthesizingAudioResult(GDBusConnection *connection,
                               const gchar *sender_name,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *signal_name,
                               GVariant *parameters,
                               gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->synthesizingAudioCallback_ == nullptr) {
        std::cout << "SynthesisAudioResult callback is nullptr" << std::endl;
        return;
    }
    proxy->synthesizingAudioCallback_(synthesisResult(parameters));
}

void onStopSynthesizingAudioResult(GDBusConnection *connection,
                                   const gchar *sender_name,
                                   const gchar *object_path,
                                   const gchar *interface_name,
                                   const gchar *signal_name,
                                   GVariant *parameters,
                                   gpointer user_data) {
    auto *proxy = static_cast<SpeechProcessorProxy *>(user_data);
    if (proxy->stopSynthesizingAudioCallback_ == nullptr) {
        std::cout << "stopSynthesizingAudio callback is nullptr" << std::endl;
        return;
    }
    proxy->stopSynthesizingAudioCallback_();
}

SpeechProcessorProxy::SpeechProcessorProxy() { init(); }

SpeechProcessorProxy::~SpeechProcessorProxy() { destroy(); }

SpeechResult SpeechProcessorProxy::initEngine(
        const std::string &engineName, const Json::Value &config,
        gpointer userData) {
    GError *error = nullptr;
    aisdk_speech_processor_call_init_sync(
        delegate_, engineName.c_str(), config.toStyledString().c_str(),
        &sessionId_, &initErrorCode_, nullptr, &error);

    if (error) {
        fprintf(stderr, "init engine error: %s\n", error->message);
        return SpeechResult::SPEECH_SERVER_ERROR;
    }
    fprintf(stdout, "proxy initEngine sessionId: %d\n", sessionId_);
    connectSignals();
    return (SpeechResult)initErrorCode_;
}

SpeechResult SpeechProcessorProxy::startRecognizingAudio(const char *audioProperties) {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    GError *error = nullptr;
    gint errorCode;
    aisdk_speech_processor_call_start_recognizing_audio_sync(
        delegate_, audioProperties, sessionId_, &errorCode, nullptr, &error);
    if (error) {
        fprintf(stderr, "start recognizing audio error: %s\n", error->message);
        return SpeechResult::SPEECH_SERVER_ERROR;
    }
    return (SpeechResult)errorCode;
}

SpeechResult SpeechProcessorProxy::inputAudioData(const char *audioData,
                                                  int audioDataLength) {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    std::string data = base64Encode(std::string(audioData, audioDataLength));
    //data.data()是编码后的数据
    //audioDataLength是原始数据的数据长度
    aisdk_speech_processor_call_input_audio_data(
        delegate_, data.data(), audioDataLength, sessionId_, nullptr, nullptr, this);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechProcessorProxy::stopRecognizingAudio() {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    aisdk_speech_processor_call_stop_recognizing_audio(
        delegate_, sessionId_, nullptr, nullptr, this);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechProcessorProxy::recognizingCompleteAudio(const char *audioProperties,
                                                            const char *audioData,
                                                            int audioDataLength) {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    std::string data = base64Encode(std::string(audioData, audioDataLength));
    //data.data()是编码后的数据
    //audioDataLength是原始数据的数据长度
    aisdk_speech_processor_call_recognizing_complete_audio(
        delegate_, audioProperties, data.data(), audioDataLength, sessionId_,
        nullptr, nullptr, this);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechProcessorProxy::startSynthesizingAudio(const char *voiceProperties) {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    GError *error = nullptr;
    gint errorCode;
    aisdk_speech_processor_call_start_synthesizing_audio_sync(
        delegate_, voiceProperties, sessionId_, &errorCode, nullptr, &error);
    if (error) {
        fprintf(stderr, "start synthesizing audio error: %s\n", error->message);
        return SpeechResult::SPEECH_SERVER_ERROR;
    }
    return (SpeechResult)errorCode;
}

SpeechResult SpeechProcessorProxy::inputContinuousText(const char *text) {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    aisdk_speech_processor_call_input_continuous_text(
        delegate_, text, sessionId_, nullptr, nullptr, this);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechProcessorProxy::stopSynthesizingAudio() {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    aisdk_speech_processor_call_stop_synthesizing_audio(
        delegate_, sessionId_, nullptr, nullptr, this);
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechProcessorProxy::recognizingCompleteText(const char *voiceProperties,
                                                           const char *text) {
    if (initErrorCode_) {
        return (SpeechResult)initErrorCode_;
    }
    aisdk_speech_processor_call_recognizing_complete_text(
        delegate_, voiceProperties, text, sessionId_, nullptr, nullptr, this);
    return SpeechResult::SPEECH_SUCCESS;
}


void SpeechProcessorProxy::init() {
    if (!ServerProxy::getInstance().available()) {
        g_printerr("Error creating speech processor proxy: Server proxy "
                   "connection is unavailable.\n");
        return;
    }
    createDelegate();
}

void SpeechProcessorProxy::destroy() {
    if (delegate_ != nullptr) {
        g_object_unref(delegate_);
    }
}

void SpeechProcessorProxy::createDelegate() {
    GError *error = nullptr;
    auto *connection = ServerProxy::getInstance().getConnection();
    delegate_ = aisdk_speech_processor_proxy_new_sync(
        connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath_,
        nullptr, &error);
    if (delegate_ == nullptr) {
        g_printerr("Error creating speech processor proxy %s: %s\n",
                   objectPath_, error->message);
        g_error_free(error);
    }
}

void SpeechProcessorProxy::connectSignals() {
    auto *connection = ServerProxy::getInstance().getConnection();

    std::string interfaceName = interfaceName_ + std::to_string(sessionId_);
    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName.c_str(), "RecognizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onRecognizingAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName.c_str(), "StopRecognizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onStopRecognizingAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName.c_str(), "SynthesizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onSynthesizingAudioResult, this, nullptr);

    g_dbus_connection_signal_subscribe(connection, nullptr,
        interfaceName.c_str(), "StopSynthesizingAudioResult", objectPath_, NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, onStopSynthesizingAudioResult, this, nullptr);
}

std::string SpeechProcessorProxy::base64Encode(const std::string &input) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}
