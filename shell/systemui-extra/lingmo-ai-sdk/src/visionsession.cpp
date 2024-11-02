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

#include "visionsession.h"
#include "vision.h"
#include <cstdio>
#include <jsoncpp/json/value.h>

namespace vision {

VisionSession::VisionSession() {}

VisionResult VisionSession::init(const std::string &engineName, const Json::Value &config) {
    return visionProcessorProxy_.initEngine(engineName, config);
}

void VisionSession::setPromptImageCallback(
        ImageResultCallback callback, std::any userData) {
    visionProcessorProxy_.setPromptImageCallback(
                [callback, userData](VisionImageData result) {
        callback(result, std::any_cast<void *>(userData));
    });
}

void VisionSession::setPromptImageSize(int width, int height) {
    visionProcessorProxy_.setPromptImageSize(width, height);
}

void VisionSession::setPromptImageNumber(int number) {
    visionProcessorProxy_.setPromptImageNumber(number);
}

void VisionSession::setPromptImageStyle(VisionImageStyle style) {
    visionProcessorProxy_.setPromptImageStyle(style);
}

std::string VisionSession::getPrompt2imageSupportedParams() const {
    return visionProcessorProxy_.getPrompt2imageSupportedParams();
}

VisionResult VisionSession::promptImage(const char* prompt) {
    if (prompt == nullptr) {
        return VisionResult::VISION_PARAM_ERROR;
    }
    return visionProcessorProxy_.promptImage(prompt);
}

}
