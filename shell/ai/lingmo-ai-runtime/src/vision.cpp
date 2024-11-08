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
#include "modelconfig.h"
#include "baiduremotemodel.h"
#include "visionsession.h"
#include <sys/socket.h>

static thread_local std::string g_vision_last_error_message;

VisionResult vision_create_session(VisionSession* session)
{
    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = new vision::BaiduSession();
        if (!baiduSession->isValid()) {
            auto errorCode = baiduSession->getResultCode();
            g_vision_last_error_message = baiduSession->lastErrorMessage();
            delete baiduSession;
            return errorCode;
        }
        *session = baiduSession;
    }

    return VISION_SUCCESS;
}

void vision_destory_session(VisionSession session)
{
    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            delete baiduSession;
        }
    }
}

void vision_set_prompt2image_callback(VisionSession session, ImageResultCallback callback, void* userData)
{
    if (!session) {
        return;
    }

    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            baiduSession->setGenerateCallback(callback);
            baiduSession->setUserData(userData);
        }
    }
}

void vision_set_prompt2image_size(VisionSession session, int width, int height)
{
    if (!session) {
        return;
    }

    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            baiduSession->setImageSize(width, height);
        }
    }
}

void vision_set_prompt2image_number(VisionSession session, int number)
{
    if (!session) {
        return;
    }

    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            baiduSession->setImageNumber(number);
        }
    }
}

void vision_set_prompt2image_style(VisionSession session, VisionImageStyle style)
{
    if (!session) {
        return;
    }

    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            baiduSession->setImageStyle(style);
        }
    }
}


VisionResult vision_prompt2image(VisionSession session, const char *prompt)
{
    if (!session) {
        return VISION_SESSION_ERROR;
    }

    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            VisionResult ret = baiduSession->generateImage(prompt);
            if (ret != VISION_SUCCESS) {
                g_vision_last_error_message = baiduSession->lastErrorMessage();
                return ret;
            }
        }
    }

    return VISION_SUCCESS;
}

const char* vision_get_prompt2image_specification(VisionSession session)
{
    if (!session) {
        return "";
    }

    if (get_default_remote_model_vendor() == BAIDU) {
        vision::BaiduSession *baiduSession = static_cast<vision::BaiduSession *>(session);
        if (baiduSession) {
            return baiduSession->getSpecification();
        }
    }

    return "";
}

const char *vision_get_last_error_message()
{
    return g_vision_last_error_message.c_str();
}
