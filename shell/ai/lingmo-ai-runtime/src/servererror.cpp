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

#include "servererror.h"
#include "util.h"

namespace {

const std::map<int, NlpResult> nlpChatErrorCodeMap = 
{
    {1, NLP_SERVER_ERROR},
    {2, NLP_SERVER_ERROR},
    {4, NLP_REQUEST_FAILED},
    {6, NLP_UNAUTHORIZED},
    {13, NLP_UNAUTHORIZED},
    {14, NLP_UNAUTHORIZED},
    {17, NLP_REQUEST_FAILED},
    {18, NLP_REQUEST_TOO_MANY},
    {19, NLP_REQUEST_FAILED},
    {100, NLP_UNAUTHORIZED},
    {110, NLP_UNAUTHORIZED},
    {111, NLP_UNAUTHORIZED},
    {200, NLP_SERVER_ERROR},
    {336000, NLP_SERVER_ERROR},
    {336007, NLP_INPUT_TEXT_LENGTH_INVAILD},
    {336100, NLP_SERVER_ERROR},
    {336103, NLP_INPUT_TEXT_LENGTH_INVAILD},
    {336501, NLP_REQUEST_TOO_MANY},
    {336502, NLP_REQUEST_TOO_MANY}
};

const std::map<int, VisionResult> visionPrompt2ImageErrorCodeMap = 
{
    {1, VISION_SERVER_ERROR},
    {2, VISION_SERVER_ERROR},
    {4, VISION_REQUEST_FAILED},
    {6, VISION_UNAUTHORIZED},
    {13, VISION_REQUEST_TOO_MANY},
    {15, VISION_REQUEST_TOO_MANY},
    {17, VISION_REQUEST_FAILED},
    {18, VISION_REQUEST_TOO_MANY},
    {19, VISION_REQUEST_FAILED},
    {100, VISION_UNAUTHORIZED},
    {110, VISION_UNAUTHORIZED},
    {111, VISION_UNAUTHORIZED},
    {500, VISION_SERVER_ERROR},
    {216100, VISION_PARAM_ERROR},
    {216303, VISION_TASK_TIMEOUT},
    {216401, VISION_REQUEST_TOO_MANY},
    {216630, VISION_SERVER_ERROR},
    {282004, VISION_GENERATE_IMAGE_BLOCKED},
    {282352, VISION_REQUEST_FAILED},
    {282501, VISION_INPUT_TEXT_LENGTH_INVAILD}
};

};

int server_error::parseErrorCode(const std::string &result)
{
    Json::Value obj = util::formatJsonFromString(result);
    if(obj.isMember("error_code") && obj.isMember("error_msg")) {
        return obj["error_code"].asInt();
    }
    return 0;
}

const NlpResult server_error::errorCode2nlpResult(int errorCode)
{
    if (nlpChatErrorCodeMap.count(errorCode) > 0) {
        return nlpChatErrorCodeMap.at(errorCode);
    }

    fprintf(stderr, "Unknown error code: %d.\n", errorCode);

    return NLP_REQUEST_FAILED;
}

const VisionResult server_error::errorCode2visionResult(int errorCode)
{
    if (visionPrompt2ImageErrorCodeMap.count(errorCode) > 0) {
        return visionPrompt2ImageErrorCodeMap.at(errorCode);
    }

    fprintf(stderr, "Unknown error code: %d.\n", errorCode);

    return VISION_REQUEST_FAILED;
}
