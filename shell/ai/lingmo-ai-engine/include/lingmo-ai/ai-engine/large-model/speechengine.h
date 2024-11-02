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

#ifndef SPEECHENGINE_H
#define SPEECHENGINE_H

#include <string>
#include <functional>
#include <vector>
#include "aiengineerror.h"

namespace ai_engine {
namespace lm {
namespace speech {

enum class ResultType {
    ContinuousRecognitionResult,
    OnceRecognitionResult,
    ContinuousSynthesisResult,
    OnceSynthesisResult,
    KeywordSpotterResult
};

struct RecognitionResult {
    std::string text;
    EngineError error;
    ResultType type;
    int speakerId;
};

struct SynthesisResult {
    std::string audioFormat;
    int audioRate;
    int audioChannel;
    std::vector<char> audioData;
    EngineError error;
    ResultType type;
};

using RecognitionResultCallback = std::function<void(RecognitionResult result)>;
using SynthesisResultCallback = std::function<void(SynthesisResult result)>;
using FinishedCallback = std::function<void(ResultType type)>;

class AbstractSpeechEngine
{
public:
    virtual ~AbstractSpeechEngine() {};
    virtual std::string backendName() = 0;
    virtual bool isCloud() = 0;

    virtual void setRecognizingCallback(RecognitionResultCallback callback) = 0;
    virtual void setRecognizedCallback(RecognitionResultCallback callback) = 0;

    virtual void setSynthesizingCallback(SynthesisResultCallback callback) = 0;
    virtual void setSynthesizedCallback(FinishedCallback callback) = 0;

    // 流式语音识别
    virtual bool initContinuousRecognitionModule(ai_engine::lm::EngineError &error) = 0;
    virtual bool startContinuousRecognition(const std::string &params, ai_engine::lm::EngineError &error) = 0;
    virtual bool writeContinuousRecognitionAudioData(const std::vector<char> &audioData, ai_engine::lm::EngineError &error) = 0;
    virtual bool stopContinuousRecognition(ai_engine::lm::EngineError &error) = 0;
    virtual bool destroyContinuousRecognitionModule(ai_engine::lm::EngineError &error) = 0;
    
    // 非流式语音识别
    virtual bool initRecognizeOnceModule(ai_engine::lm::EngineError &error) = 0;
    virtual bool recognizeOnce(const std::string &params, const std::vector<char> &audioData, ai_engine::lm::EngineError &error) = 0;
    virtual bool destroyRecognizeOnceModule(ai_engine::lm::EngineError &error) = 0;

    // 流式语音合成
    virtual bool initContinuousSynthesisModule(ai_engine::lm::EngineError &error) = 0;
    virtual bool startContinuousSynthesis(const std::string &params, ai_engine::lm::EngineError &error) = 0;
    virtual bool writeContinuousSynthesisText(const std::string &text, ai_engine::lm::EngineError &error) = 0;
    virtual bool stopContinuousSynthesis(ai_engine::lm::EngineError &error) = 0;
    virtual bool destroyContinuousSynthesisModule(ai_engine::lm::EngineError &error) = 0;

    // 非流式语音合成
    virtual bool initSynthesizeOnceModule(ai_engine::lm::EngineError &error) = 0;
    virtual bool synthesizeOnce(const std::string &params, const std::string &text, ai_engine::lm::EngineError &error) = 0;
    virtual bool destroySynthesizeOnceModule(ai_engine::lm::EngineError &error) = 0;
};

}
}
}


#endif // SPEECHENGINE_H
