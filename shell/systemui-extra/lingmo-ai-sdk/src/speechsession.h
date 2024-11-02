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

#ifndef SPEECHSESSION_H
#define SPEECHSESSION_H

#include "speech.h"
#include "services/speechprocessorproxy.h"
#include "services/keywordrecognizerproxy.h"
#include <jsoncpp/json/value.h>
#include <unordered_map>
#include <any>

namespace speech {

class SpeechSession {
public:
    SpeechSession();
    SpeechResult init(
        const std::string &engineName, const Json::Value &config);
    void setRecognizingCallback(
        SpeechRecognitionResultCallback callback, std::any userData);

    void setRecognizedCallback(
        SpeechRecognitionResultCallback callback, std::any userData);

    void setSynthesizingCallback(
        SpeechSynthesisResultCallback callback, std::any userData);

    void setSynthesizedCallback(
        SpeechFinishedCallback callback, std::any userData);

    void setKeywordRecognizedCallback(SpeechRecognitionResultCallback callback, std::any userData);

    SpeechResult startContinuousRecognition(const char* params);
    SpeechResult writeContinuousAudioData(const void* audio_data,
                                          unsigned int audio_data_length);
    SpeechResult stopContinuousRecognition();
    SpeechResult speechRecognizeOnce(const char* params,
                                     const void* audio_data,
                                     unsigned int audio_data_length);

    SpeechResult startContinuousSynthesis(const char* params);
    SpeechResult writeContinuousText(const char* text);
    SpeechResult stopContinuousSynthesis();
    SpeechResult speechSynthesizeOnce(const char* params, const char* text);

    SpeechResult startKeywordRecognizer();
    SpeechResult stopKeywordRecognizer();

private:
    enum class WorkStage {
        Recognizing,
        Recognized,
        Synthesizing,
        Synthesized
    };

    struct CallbackInfo {
        std::any callback;
        std::any userData;
    };
    using Callbacks = std::unordered_map<WorkStage, CallbackInfo>;

private:
    Callbacks callbacks_;

    SpeechProcessorProxy speechProcessorProxy_;
    KeywordRecognizerProxy keywordRecognizerProxy_;
};

}


#endif // SPEECHSESSION_H
