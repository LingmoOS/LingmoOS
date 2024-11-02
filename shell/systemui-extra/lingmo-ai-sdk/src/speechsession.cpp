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

#include "speechsession.h"
#include <jsoncpp/json/value.h>

namespace speech {

SpeechSession::SpeechSession() {}

SpeechResult SpeechSession::init(
        const std::string &engineName, const Json::Value &config) {

    //初始化关键字识别功能，接口加载模型时间会比较久
//    keywordRecognizerProxy_.initKeywordRecognizer();

    return speechProcessorProxy_.initEngine(
                engineName, config, nullptr);
}

void SpeechSession::setRecognizingCallback(
    SpeechRecognitionResultCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Recognizing] = { callback, userData };

    speechProcessorProxy_.setRecognizingAudioCallback(
                [callback, userData](RecognitionResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

void SpeechSession::setRecognizedCallback(
    SpeechRecognitionResultCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Recognized] = { callback, userData };
    speechProcessorProxy_.setStopRecognizingAudioCallback(
                [callback, userData](RecognitionResult result) {
       callback(result, std::any_cast<void *>(userData));
    });
}

void SpeechSession::setSynthesizingCallback(
    SpeechSynthesisResultCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Synthesizing] = { callback, userData };

    speechProcessorProxy_.setSynthesizingAudioCallback(
                [callback, userData](SynthesisResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

void SpeechSession::setSynthesizedCallback(
    SpeechFinishedCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Synthesized] = { callback, userData };
    speechProcessorProxy_.setStopSynthesizingAudioCallback(
                [callback, userData]() {
        callback(std::any_cast<void *>(userData));
    });
}

SpeechResult SpeechSession::startContinuousRecognition(const char* params) {
    if (params == nullptr) {
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    return speechProcessorProxy_.startRecognizingAudio(params);
}

SpeechResult SpeechSession::writeContinuousAudioData(const void* audio_data,
                                                     unsigned int audio_data_length) {
    if (audio_data == nullptr || audio_data_length == 0) {
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    return speechProcessorProxy_.inputAudioData((char*)audio_data, audio_data_length);
}

SpeechResult SpeechSession::stopContinuousRecognition() {
    return speechProcessorProxy_.stopRecognizingAudio();
}

SpeechResult SpeechSession::speechRecognizeOnce(const char* params,
                                                const void* audio_data,
                                                unsigned int audio_data_length) {
    if (params == nullptr || audio_data == nullptr || audio_data_length == 0) {
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    return speechProcessorProxy_.recognizingCompleteAudio(
        params, (char*)audio_data, audio_data_length);
}

SpeechResult SpeechSession::startContinuousSynthesis(const char* params) {
    if (params == nullptr) {
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    return speechProcessorProxy_.startSynthesizingAudio(params);
}

SpeechResult SpeechSession::writeContinuousText(const char* text) {
    if (text == nullptr) {
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    return speechProcessorProxy_.inputContinuousText(text);
}

SpeechResult SpeechSession::stopContinuousSynthesis() {
    return speechProcessorProxy_.stopSynthesizingAudio();
}

SpeechResult SpeechSession::speechSynthesizeOnce(const char* params,
                                                 const char* text) {
    if (params == nullptr || text == nullptr) {
        return SpeechResult::SPEECH_PARAM_ERROR;
    }
    return speechProcessorProxy_.recognizingCompleteText(params, text);
}

void SpeechSession::setKeywordRecognizedCallback(SpeechRecognitionResultCallback callback, std::any userData) {
    callbacks_[SpeechSession::WorkStage::Recognizing] = { callback, userData };
    keywordRecognizerProxy_.setRecognizedCallback([callback, userData](RecognitionResult result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

SpeechResult SpeechSession::startKeywordRecognizer() {
    keywordRecognizerProxy_.startKeywordRecognizer();
    return SpeechResult::SPEECH_SUCCESS;
}

SpeechResult SpeechSession::stopKeywordRecognizer() {
    keywordRecognizerProxy_.stopKeywordRecognizer();
    return SpeechResult::SPEECH_SUCCESS;
}

}


