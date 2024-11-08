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

#include <iostream>
#include <fstream>
#include <assert.h>
#include "engine/aienginepluginmanager.h"
#include <lingmo-ai/ai-engine/large-model/cloudspeechengine.h>
#include <lingmo-ai/ai-engine/large-model/cloudnlpengine.h>
#include <lingmo-ai/ai-engine/large-model/cloudvisionengine.h>

std::vector<char> readAudioData(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> audioData(fileSize);
    file.read(audioData.data(), fileSize);
    return audioData;
}

void testSpeechOfBaidu() {
    ai_engine::AiEnginePluginManager aepm;
    auto baiduEngine = aepm.createSpeechEngine("baidu");

    ai_engine::lm::speech::AbstractCloudSpeechEngine *speechEnginePtr(
        dynamic_cast<ai_engine::lm::speech::AbstractCloudSpeechEngine *>(baiduEngine.release()));

    std::unique_ptr<ai_engine::lm::speech::AbstractCloudSpeechEngine> speechEngine(speechEnginePtr);

    speechEngine->setRecognizingCallback([] (ai_engine::lm::speech::RecognitionResult result){
        std::cout << result.text << std::endl;
    });
    speechEngine->setRecognizedCallback([] (ai_engine::lm::speech::ResultType type){
        std::cout << "Recognize finished..." << std::endl;
    });

    speechEngine->setSynthesizingCallback([] (ai_engine::lm::speech::SynthesisResult result){
        std::cout << result.audioData.size() << std::endl;
    });
    speechEngine->setSynthesizedCallback([] (ai_engine::lm::speech::ResultType type){
        std::cout << "Synthesize finished..." << std::endl;
    });

    speechEngine->setAppId("55263856");
    speechEngine->setApiKey("wna5LFOV7qK4fWxvwP7I0FtD");
    speechEngine->setSecretKey("S15zv3R9MLQOZiLnJtRV3V2FNq3TM4ps");

    auto audioData = readAudioData("/home/linculn/16k_10.pcm");
    auto ret = speechEngine->recognizeOnce("\{\"rate\": 1600}", audioData);
    std::cout << "recongnize once: " << (int)ret << std::endl;
    assert(ret == ai_engine::ErrorCode::Success);

    std::string text = "灵墨软件，风行天下";
    ret = speechEngine->synthesizeOnce("\{\"speed\": 5}", text);
    std::cout << "synthesize once: " << (int)ret << std::endl;
    assert(ret == ai_engine::ErrorCode::Success);

    audioData = readAudioData("16k.pcm");
    ret = speechEngine->recognizeOnce("\{\"rate\": 1600}", audioData);
    std::cout << "recongnize once: " << (int)ret << std::endl;
}

void testNlpOfBaidu() {
    ai_engine::AiEnginePluginManager aepm;
    auto baiduEngine = aepm.createNlpEngine("baidu");

    ai_engine::lm::nlp::AbstractCloudNlpEngine *nlpEnginePtr(
        dynamic_cast<ai_engine::lm::nlp::AbstractCloudNlpEngine *>(baiduEngine.release()));

    std::unique_ptr<ai_engine::lm::nlp::AbstractCloudNlpEngine> nlpEngine(nlpEnginePtr);

    nlpEngine->setAppId("45358279");
    nlpEngine->setApiKey("6i78SlJ3qdWFhmK3gjS8Gn5D");
    nlpEngine->setSecretKey("kLQAdbnAQC0zSbJ87BqCxOjy4ObY8cqL");
    nlpEngine->setChatResultCallback([] (const std::string& result)->void{
        std::cout << result << std::endl;
    });

    auto ret = nlpEngine->chat("我叫灵墨软件");
    std::cout << "chat: " << (int)ret << std::endl;
    assert(ret == ai_engine::ErrorCode::Success);

    ret = nlpEngine->chat("我是谁");
    std::cout << "chat: " << (int)ret << std::endl;
    assert(ret == ai_engine::ErrorCode::Success);
}

void testVisionOfBaidu() {
    ai_engine::AiEnginePluginManager aepm;
    auto baiduEngine = aepm.createVisionEngine("baidu");

    ai_engine::lm::vision::AbstractCloudVisionEngine *visionEnginePtr(
        dynamic_cast<ai_engine::lm::vision::AbstractCloudVisionEngine *>(baiduEngine.release()));

    std::unique_ptr<ai_engine::lm::vision::AbstractCloudVisionEngine> visionEngine(visionEnginePtr);

    visionEngine->setAppId("45358279");
    visionEngine->setApiKey("iHVewZRPlIuvYsFImmXnVykh");
    visionEngine->setSecretKey("5SEApYcfKCTL8xvHTS60IW8JkbNx4ZIk");
    visionEngine->setPrompt2ImageNumber(2);
    visionEngine->setPrompt2ImageCallback([] (ai_engine::lm::vision::ImageData imageData)->void{
        std::cout << "format: " << imageData.format << std::endl;
        std::cout << "height: " << imageData.height << std::endl;
        std::cout << "width: " << imageData.width << std::endl;
        std::cout << "index: " << imageData.index << std::endl;
        std::cout << "total: " << imageData.total << std::endl;
        std::cout << "image data size: " << imageData.imageData.size() << std::endl;
    });

    auto ret = visionEngine->prompt2Image("黄金比例构成，一颗冰晶小苍兰，微距摄影，超详细刻画");
    std::cout << "vision: " << (int)ret << std::endl;
    assert(ret == ai_engine::ErrorCode::Success);
}

void testBaiduEngine() {
    testSpeechOfBaidu();
    testNlpOfBaidu();
    testVisionOfBaidu();
}

int main(int argc, char* argv[]) {
    testBaiduEngine();
    return 0;
}
