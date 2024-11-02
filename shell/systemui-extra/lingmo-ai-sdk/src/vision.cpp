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

#include "vision.h"
#include "visionsession.h"
#include <cstdio>
#include <sys/socket.h>
#include "settings/globalsettings.h"

static thread_local std::string g_vision_last_error_message;

VisionResult vision_create_session(VisionSession* session) {
    vision::VisionSession *visionSession = new vision::VisionSession();
    *session = visionSession;
    return VISION_SUCCESS;
}

void vision_destroy_session(VisionSession session) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (visionSession) {
        delete visionSession;
    }
}

VisionResult vision_init_session(VisionSession session) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return VISION_SESSION_ERROR;
    }
    const auto& visionSettings = aisdk::GlobalSettings::getInstance().getChildSettings(CAPABILITY_VISION);

    const std::string model = visionSettings.getModel(visionSettings.getDeployPolicy());
    fprintf(stdout, "Vision deploy policy:%i,  model name: %s\n",
            (int)visionSettings.getDeployPolicy(), model.c_str());

    auto config = visionSettings.getModelConfig(visionSettings.getDeployPolicy(), model);
    return visionSession->init(model, config);
}

void vision_set_prompt2image_callback(
        VisionSession session, ImageResultCallback callback, void* userData) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return;
    }
    visionSession->setPromptImageCallback(callback, userData);
}

void vision_set_prompt2image_size(VisionSession session, int width, int height) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return;
    }
    visionSession->setPromptImageSize(width, height);
}

void vision_set_prompt2image_number(VisionSession session, int number) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return;
    }
    visionSession->setPromptImageNumber(number);
}

void vision_set_prompt2image_style(VisionSession session, VisionImageStyle style) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return;
    }
    visionSession->setPromptImageStyle(style);
}


VisionResult vision_prompt2image(VisionSession session, const char *prompt) {
    return VISION_SESSION_ERROR;
}

VisionResult vision_prompt2image_async(VisionSession session, const char *prompt) {
    if (prompt == nullptr) {
        fprintf(stderr, "vision prompt image error: prompt is nullptr!\n");
        return VISION_PARAM_ERROR;
    }
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return VISION_SESSION_ERROR;
    }
    return visionSession->promptImage(prompt);
}

const char *vision_get_prompt2image_supported_params(VisionSession session) {
    vision::VisionSession *visionSession = static_cast<vision::VisionSession *>(session);
    if (!visionSession) {
        return nullptr;
    }

    static std::string params;
    if (params.empty()) {
        params = visionSession->getPrompt2imageSupportedParams();
    }

    return params.c_str();
}

const char *vision_get_last_error_message() {
    return g_vision_last_error_message.c_str();
}

